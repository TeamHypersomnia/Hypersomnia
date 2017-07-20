#pragma once
#include <type_traits>
#include <tuple>

#include "augs/templates/predicate_templates.h"
#include "augs/templates/type_list.h"

template <class, class>
struct prepend_to_list;

template <class T, template <class...> class List, class... Args>
struct prepend_to_list<T, List<Args...>> {
	using type = List<T, Args...>;
};

template <class Element, class List>
using prepend_to_list_t = typename prepend_to_list<Element, List>::type;

template <class, class>
struct append_to_list;

template <class T, template <class...> class List, class... Args>
struct append_to_list<T, List<Args...>> {
	using type = List<Args..., T>;
};

template <class Element, class List>
using append_to_list_t = typename append_to_list<Element, List>::type;


template <size_t, class>
struct add_to_sequence;

template <size_t I, size_t... Is>
struct add_to_sequence<I, std::index_sequence<Is...>> {
	using type = std::index_sequence<I, Is...>;
};

template <size_t I, class T>
struct sequence_element;

template <size_t I, class T, T Head, T... Tail>
struct sequence_element<I, std::integer_sequence<T, Head, Tail...>>
	: sequence_element<I - 1, std::integer_sequence<T, Tail...>> { };

template <class T, T Head, T... Tail>
struct sequence_element<0, std::integer_sequence<T, Head, Tail...>> {
	static constexpr T value = Head;
};

template <size_t I, class T>
static constexpr auto sequence_element_v = sequence_element<I, T>::value;

template <
	size_t Index,
	template <class...> class Criterion,
	class List
>
struct filter_types_detail;

template <
	size_t Index,
	template <class...> class Criterion,
	template <class...> class List
>
struct filter_types_detail<
	Index,
	Criterion,
	List<>
> { 
	using type = std::tuple<>; 
	using indices = std::index_sequence<>;
};

template <
	size_t Index,
	template <class...> class Criterion,
	template <class...> class List,
	class Head,
	class... Tail
>
struct filter_types_detail<
	Index,
	Criterion,
	List<Head, Tail...>
> {
	using type = std::conditional_t<
		Criterion<Head>::value,
		prepend_to_list_t<
			Head,
			typename filter_types_detail<Index + 1, Criterion, List<Tail...>>::type
		>,
		typename filter_types_detail<Index + 1, Criterion, List<Tail...>>::type
	>;

	using indices = std::conditional_t<
		Criterion<Head>::value,
		typename add_to_sequence<
			Index,
			typename filter_types_detail<Index + 1, Criterion, List<Tail...>>::indices
		>::type,
		typename filter_types_detail<Index + 1, Criterion, List<Tail...>>::indices
	>;

	static constexpr bool found = indices().size() > 0;

	template <size_t I, class = void>
	struct get_type {

	};

	template <size_t I>
	struct get_type<I, std::enable_if_t<I < std::tuple_size_v<type>>> {
		using type = std::tuple_element_t<I, type>;
	};
};


template <
	template <class...> class Criterion,
	class List
>
using filter_types_in_list = filter_types_detail<0, Criterion, List>;


template <
	template <class...> class Criterion,
	class List
>
using filter_types_in_list_t = typename filter_types_detail<0, Criterion, List>::type;

template <
	template <class...> class Criterion,
	class... Args
>
using filter_types = filter_types_in_list_t<Criterion, type_list<Args...>>;

template <
	template <class...> class Criterion,
	class List
>
using find_matching_type_in_list = typename filter_types_in_list<Criterion, List>::template get_type<0>::type;

template <class S, class List>
constexpr bool is_one_of_list_v = filter_types_in_list<bind_types_t<std::is_same, S>, List>::found;

template <class S, class... Types>
constexpr bool is_one_of_v = is_one_of_list_v<S, type_list<Types...>>;

template <class S, class List>
using is_one_of_list = std::bool_constant<is_one_of_list_v<S, List>>;

template <class S, class... Types>
using is_one_of = is_one_of_list<S, type_list<Types...>>;

template <class S, class List>
constexpr size_t index_in_list_v = sequence_element_v<0, typename filter_types_in_list<bind_types_t<std::is_same, S>, List>::indices>;

template <class S, class... Types>
constexpr size_t index_in_v = index_in_list_v<S, type_list<Types...>>;

template <class S, class List>
constexpr size_t count_occurences_in_list_v = typename filter_types_in_list<bind_types_t<std::is_same, S>, List>::indices::size();

template <class S, class... Types>
constexpr size_t count_occurences_in_v = count_occurences_in_list_v<S, type_list<Types...>>;

template <class S, class List>
using find_convertible_type_in_list_t = find_matching_type_in_list<bind_types_t<std::is_convertible, S>, List>;

template <class S, class... Types>
using find_convertible_type_in_t = find_convertible_type_in_list_t<S, std::tuple<Types...>>;

template <size_t I, class... Types>
using nth_type_in_t = typename std::tuple_element<I, std::tuple<Types...>>::type;

template <class T, class Candidate>
struct is_key_type_equal_to : std::bool_constant<std::is_same_v<T, typename Candidate::key_type>> {

};

template <class SearchedKeyType, class List>
using find_type_with_key_type_in_list_t = find_matching_type_in_list<bind_types_t<is_key_type_equal_to, SearchedKeyType>, List>;

template <class SearchedKeyType, class... Types>
using find_type_with_key_type_t = find_type_with_key_type_in_list_t<SearchedKeyType, type_list<Types...>>;

template <class T, class ContainerList>
decltype(auto) get_container_with_key_type(ContainerList&& containers) {
	return std::get<
		find_type_with_key_type_in_list_t<
			T, 
			std::decay_t<ContainerList>
		>
	> (
		std::forward<ContainerList>(containers)
	);
}


template<class T>
struct always_false : std::false_type {};

template<class T>
constexpr auto always_false_v = always_false<T>;