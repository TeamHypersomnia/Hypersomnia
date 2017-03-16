#pragma once
#include <type_traits>

namespace templates_detail {
	template <typename T>
	struct identity {
		typedef T type;
	};
}

template <
	template<class, class> class Criterion, 
	typename SearchedType, 
	typename... Candidates
>
struct find_matching_type;

template <
	template<class, class> class Criterion, 
	typename SearchedType
>
struct find_matching_type<Criterion, SearchedType> {

};

template <
	template<class, class> class Criterion, 
	typename SearchedType, 
	typename Candidate, 
	typename... Candidates
>
struct find_matching_type<
	Criterion,
	SearchedType, 
	Candidate, 
	Candidates...
> :
	std::conditional_t<
		Criterion<SearchedType, Candidate>::value, 
		::templates_detail::identity<Candidate>,
		find_matching_type<
			Criterion, 
			SearchedType, 
			Candidates...
		>
	> 
{
};

template <
	template<class, class> class Criterion,
	typename SearchedType,
	typename... Candidates
>
struct found_matching_type_detail {
private:
	template <class, class = void>
	struct result : std::false_type {

	};
	
	template <class Dummy>
	struct result<
		Dummy,
		decltype(
			std::declval<
				typename find_matching_type<Criterion, SearchedType, Candidates...>::type
			>,
			void()
		)
	> : std::true_type 
	{
	};
public:
	static constexpr bool value = result<void>::value;
};

template <
	template<class, class> class Criterion,
	typename SearchedType,
	typename... Candidates
>
struct found_matching_type 
	: std::bool_constant<
		found_matching_type_detail<Criterion, SearchedType, Candidates...>::value
	>
{
};

template <class S, class... Types>
constexpr bool is_one_of_v = found_matching_type<std::is_same, S, Types...>::value;

template<class S, class... Types>
using find_convertible_type_t = typename find_matching_type<std::is_convertible, S, Types...>::type;
