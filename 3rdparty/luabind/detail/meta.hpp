// Copyright Michael Steinberg 2013. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_META_HPP_INCLUDED
#define LUABIND_META_HPP_INCLUDED

#include <tuple>

namespace luabind { namespace meta
{
	struct type_list_tag {};
	struct index_list_tag {};

	/*
	Index list and type list share pretty common patterns... is there a way to unify them?
	*/

	template< unsigned int >
	struct count
	{};

	template< unsigned int >
	struct index
	{};

	template< typename Type >
	struct type
	{};

	// Use this to unpack a parameter pack into a list of T's
	template< typename T, typename DontCare >
	struct unpack_helper
	{
		using type =  T;
	};
	
	struct init_order {
		init_order(std::initializer_list<int>) {}
	};

	// common operators
	template< typename T >
	struct size;

	template< typename T, unsigned int Index >	// Specializations so index lists can use the same syntax
	struct get;

	template< typename... Lists >
	struct join;

	template< typename List1, typename List2, typename... Lists >
	struct join< List1, List2, Lists... > {
		// Could try to join on both sides
		using type = typename join< typename join< List1, List2 >::type, Lists... >::type;
	};

	// convenience
	template< typename T >
	struct front : public get< T, 0 >
	{
	};

	template< typename List, typename T >
	struct push_front;

	template< typename List, typename T >
	struct push_back;

	template< typename T >
	struct pop_front;

	template< typename T >
	struct pop_back;

	template< typename List, unsigned int Index, typename T >
	struct replace;

	template< typename List, unsigned int Index, template< typename > class T >
	struct enwrap;

	template< typename List, template< typename > class T >
	struct enwrap_all;

	template< typename List, unsigned int Index, template< typename > class T >
	struct transform;

	template< typename List, template< typename > class T >
	struct transform_all;

	template< typename T, unsigned int start, unsigned int end >
	struct sub_range;

	// Used as terminator on type and index lists
	struct null_type {};
	
	template< typename... Types >
	struct type_list : public type_list_tag
	{
		template< unsigned int Index >
		using at = typename get<type_list,Index>::type;
	};

	template< typename... Types1, typename... Types2 >
	type_list<Types1..., Types2...> operator|(const type_list<Types1...>&, const type_list<Types2...>&) {
		return type_list<Types1..., Types2...>();
	}
	
	template< typename T >
	struct is_typelist : public std::false_type
	{
		static const bool value = false;
	};

	template< typename... Types >
	struct is_typelist< type_list< Types... > > : public std::true_type
	{
		static const bool value = true;
	};

	/*
	Find type
	*/

	template< typename TypeList, typename Type >
	struct contains;

	template< typename Type0, typename... Types, typename Type >
	struct contains< type_list<Type0, Types...>, Type >
		: std::conditional< std::is_same<Type0, Type>::value, std::true_type, contains< type_list<Types...>, Type > >::type
	{
	};

	template< typename Type >
	struct contains< type_list< >, Type >
		: std::false_type
	{
	};

	/*
	size
	*/

	template< >
	struct size< type_list< > >  {
		enum { value = 0 };
	};

	template< typename Type0, typename... Types >
	struct size< type_list< Type0, Types... > >  {
		enum { value = 1 + size< type_list<Types...> >::value };
	};


	template< typename... Types, typename Type >
	struct push_front< type_list<Types...>, Type >
	{
		using type = type_list< Type, Types... >;
	};

	template< typename... Types, typename Type >
	struct push_back< type_list<Types...>, Type >
	{
		using type = type_list< Types..., Type >;
	};

	/*
	pop_front
	*/

	template< typename Type0, typename... Types >
	struct pop_front< type_list< Type0, Types... > >
	{
		using type = type_list< Types... >;
	};

	template< >
	struct pop_front< type_list< > >
	{
		using type = type_list< >;
	};

	/*
	pop_back
	*/

    // Empty case
    template< >
    struct pop_back< type_list< > >
    {
        using type = type_list< >;
    };

    // Single element case
    template< typename TypeN> struct pop_back< type_list< TypeN > >
    {
        typedef type_list<> type; // TypeN will be removed
    };

    // Normal case, recursively pop_back while pushing all but one onto the front
    template< typename TypeN, typename... Types >
    struct pop_back< type_list< TypeN, Types... > >
    {
        typedef typename pop_back< type_list<Types...> >::type tail;
        typedef typename push_front<tail, TypeN>::type type;
    };

    /*
    get
    */

	template< typename Element0, typename... Elements, unsigned int Index >
	struct get< type_list<Element0, Elements...>, Index > {
		using type = typename get< type_list<Elements...>, Index - 1 >::type;
	};

	template< typename Element0, typename... Elements >
	struct get< type_list<Element0, Elements...>, 0 >
	{
		using type = Element0;
	};

	template< unsigned int Index >
	struct get< type_list< >, Index >
	{
		static_assert(size< type_list< int > >::value == 1, "Bad Index");
	};

	template< typename... Types1, typename... Types2 >
	struct join< type_list< Types1... >, type_list< Types2... > >
	{
		using type = type_list< Types1..., Types2... >;
	};

	/*
	template< typename List, unsigned int Index, typename T >
	struct replace;
	*/

	namespace detail {
		template< typename HeadList, typename TailList, typename Type, unsigned int Index >
		struct replace_helper;

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, typename Type, unsigned int Index >
		struct replace_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, Index> {
			using type = typename replace_helper< type_list< HeadTypes..., CurrentType >, type_list<TailTypes...>, Type, Index - 1 >::type;
		};

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, typename Type >
		struct replace_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, 0 > {
			using type = type_list< HeadTypes..., Type, TailTypes... >;
		};
	}

	template< typename... Types, unsigned int Index, typename Type >
	struct replace< type_list< Types... >, Index, Type >
	{
		using TypeList = type_list< Types... >;

		using type = typename meta::join< 
						typename meta::sub_range< TypeList, 0, Index >::type, meta::type_list<Type>,
						typename meta::sub_range< TypeList, Index + 1, sizeof...(Types) >::type 
					>::type;
	};

	namespace detail {
		template< typename HeadList, typename TailList, template< typename > class Type, unsigned int Index >
		struct enwrap_helper;

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, template< typename > class Type, unsigned int Index >
		struct enwrap_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, Index> {
			using type = typename enwrap_helper< type_list< HeadTypes..., CurrentType >, type_list<TailTypes...>, Type, Index - 1 >::type;
		};

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, template< typename > class Type >
		struct enwrap_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, 0> {
			using type = type_list< HeadTypes..., Type<CurrentType>, TailTypes... >;
		};
	}

	template< typename... Types, unsigned int Index, template< typename >  class Type >
	struct enwrap< type_list< Types... >, Index, Type > {
		using type = typename detail::enwrap_helper< type_list< >, type_list< Types... >, Type, Index >::type;
	};

	template< typename... Types, template< typename > class Enwrapper >
	struct enwrap_all< type_list< Types... >, Enwrapper >
	{
		using type = type_list< Enwrapper< Types >... >;
	};

	namespace detail {
		template< typename HeadList, typename TailList, template< typename > class Type, unsigned int Index >
		struct transform_helper;

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, template< typename > class Type, unsigned int Index >
		struct transform_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, Index> {
			using type = typename transform_helper< type_list< HeadTypes..., CurrentType >, type_list<TailTypes...>, Type, Index - 1 >::type;
		};

		template< typename... HeadTypes, typename CurrentType, typename... TailTypes, template< typename > class Type >
		struct transform_helper< type_list< HeadTypes... >, type_list< CurrentType, TailTypes... >, Type, 0> {
			using type = type_list< HeadTypes..., typename Type<CurrentType>::type, TailTypes... >;
		};
	}

	template< typename... Types, unsigned int Index, template< typename >  class Type >
	struct transform< type_list< Types... >, Index, Type > {
		using type = typename detail::transform_helper< type_list< >, type_list< Types... >, Type, Index >::type;
	};


	template< typename Type0, typename... Types, template< typename >  class Type >
	struct transform_all< type_list< Type0, Types... >, Type > {
		using type = typename push_front< typename transform_all< type_list<Types...>, Type >::type, typename Type<Type0>::type >::type;
	};

	template< template< typename >  class Type >
	struct transform_all< type_list< >, Type > {
		using type = type_list< >;
	};


	/*
	Tuple from type list
	*/
	template< class TypeList >
	struct make_tuple;

	template< typename... Types >
	struct make_tuple< type_list< Types... > >
	{
		using type = std::tuple< Types... >;
	};


	/*
	Type selection
	*/

	template< typename ConvertibleToTrueFalse, typename Result >
	struct case_ : public ConvertibleToTrueFalse {
		using type = Result;
	};

	template< typename Result >
	struct default_ {
		using type = Result;
	};


	template< typename Case, typename... CaseList >
	struct select_
	{
		using type = typename std::conditional< 
								std::is_convertible<Case, std::true_type>::value,
								typename Case::type, 
								typename select_<CaseList...>::type 
							  >::type;
	};

	template< typename Case >
	struct select_< Case >
	{
		using type = typename std::conditional<
								std::is_convertible<Case, std::true_type>::value,
								typename Case::type,
								null_type 
							  >::type;
	};

	template< typename T >
	struct select_< default_<T> > {
		using type = typename default_<T>::type;
	};

	/*
	Create index lists to expand on type_lists
	*/

	template< unsigned int... Indices >
	struct index_list : public index_list_tag
	{
	};

	/*
	Index index list
	*/

	template< unsigned int Index0, unsigned int... Indices, unsigned int Index >
	struct get< index_list<Index0, Indices...>, Index > {
		enum { value = get< index_list<Indices...>, Index - 1 >::value };
	};

	template< unsigned int Index0, unsigned int... Indices >
	struct get< index_list<Index0, Indices...>, 0 >
	{
		enum { value = Index0 };
	};

	template< unsigned int Index >
	struct get< index_list< >, Index >
	{
		static_assert(size< index_list< Index > >::value == 1, "Bad Index");
	};

	template< >
	struct get< index_list< >, 0 >
	{
	};

	/*
	Index list size
	*/

	template< >
	struct size< index_list< > >  {
		enum { value = 0 };
	};

	template< unsigned int Index0, unsigned int... Indices >
	struct size< index_list< Index0, Indices... > >  {
		enum { value = 1 + size< index_list< Indices... > >::value };
	};

	template< unsigned int... Indices, unsigned int Index >
	struct push_front< index_list< Indices... >, index<Index> >
	{
		using type = index_list< Index, Indices... >;
	};

	template< unsigned int... Indices, unsigned int Index >
	struct push_back< index_list< Indices... >, index<Index> >
	{
		using type = index_list< Indices..., Index >;
	};

	/*
	pop_front
	*/

	template< unsigned int Index0, unsigned int... Indices >
	struct pop_front< index_list< Index0, Indices... > > {
		using type = index_list< Indices... >;
	};

	template< >
	struct pop_front< index_list< > > {
		using type = index_list<  >;
	};


	namespace detail {

		template< unsigned int curr, unsigned int end, unsigned int... Indices >
		struct make_index_range :
			public make_index_range< curr + 1, end, Indices..., curr >
		{
			static_assert(end >= curr, "end must be greater or equal to start");
		};

		template< unsigned int end, unsigned int... Indices >
		struct make_index_range< end, end, Indices... >
		{
			using type = index_list< Indices... >;
		};

	}

	template< unsigned int start, unsigned int end >
	struct make_index_range {
		static_assert(end >= start, "end must be greater than or equal to start");
		using type = typename detail::make_index_range< start, end >::type;
	};

	template< unsigned int start, unsigned int end >
	using index_range = typename make_index_range<start, end>::type;

	/*
	Exracts the first N elements of an index list and creates a new index list from them
	*/

	namespace detail {

		template< typename SourceList, typename IndexList >
		struct sub_range_index;

		template< typename SourceList, unsigned int... Indices >
		struct sub_range_index< SourceList, index_list< Indices... > > {
			using type = index_list< get< SourceList, Indices >::value... >;
		};

		template< typename SourceList, typename IndexList >
		struct sub_range_type;

		template< typename SourceList, unsigned int... Indices >
		struct sub_range_type< SourceList, index_list< Indices... > > {
			using type = type_list< typename get< SourceList, Indices >::type... >;
		};

	}

	template< unsigned int start, unsigned int end, unsigned int... Indices >
	struct sub_range< index_list<Indices...>, start, end >
	{
		static_assert(end >= start, "end must be greater or equal to start");
		using type = typename detail::sub_range_index< index_list<Indices...>, typename make_index_range<start, end>::type >::type;
	};

	template< unsigned int start, unsigned int end, typename... Types >
	struct sub_range< type_list<Types...>, start, end >
	{
		static_assert(end >= start, "end must be greater or equal to start");
		using type = typename detail::sub_range_type< type_list<Types...>, typename make_index_range<start, end>::type >::type;
	};

	template< typename IndexList, unsigned int Index >
	struct push_back_index;

	template< unsigned int... Indices, unsigned int Index >
	struct push_back_index< index_list< Indices... >, Index >
	{
		using type = index_list< Indices..., Index >;
	};


	template< typename T >
	struct sum;
	
	template< unsigned int Arg0, unsigned int... Args >
	struct sum< index_list<Arg0, Args...> >
	{
		enum{ value = Arg0 + sum<index_list<Args...>>::value };
	};

	template< >
	struct sum< index_list< > > {
		enum {value = 0};
	};

	/*
		and_ or_
	*/

	template< typename... ConvertiblesToTrueFalse >
	struct and_;

	template< typename Convertible0, typename... Convertibles >
	struct and_< Convertible0, Convertibles... >
		: std::conditional <
		std::is_convertible< Convertible0, std::true_type >::value,
		and_< Convertibles... >,
		std::false_type > ::type
	{
	};

	template< >
	struct and_< >
		: std::true_type
	{
	};


	template< typename... ConvertiblesToTrueFalse >
	struct or_;

	template< typename Convertible0, typename... Convertibles >
	struct or_< Convertible0, Convertibles... >
		: std::conditional <
		std::is_convertible< Convertible0, std::true_type >::value,
		std::true_type,
		or_< Convertibles... > 
		> ::type
	{
	};

	template< >
	struct or_< >
		: std::true_type
	{
	};

}}

#endif

