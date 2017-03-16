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
struct found_matching_type {
private:
	typedef char yes;
	typedef long no;

	template <typename C>
	static yes check(typename C::type*);

	template <typename C>
	static no check(...);

public:
	static constexpr bool value = sizeof(check<find_matching_type<Criterion, SearchedType, Candidates...>>(0)) == sizeof(yes);
};

template <class S, class... Types>
constexpr bool is_one_of_v = typename found_matching_type<std::is_same, S, Types...>::value;

template<class S, class... Types>
using find_convertible_type_t = typename find_matching_type<std::is_convertible, S, Types...>::type;
