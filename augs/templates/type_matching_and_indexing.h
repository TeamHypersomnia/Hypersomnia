#pragma once
#include <type_traits>
#include <tuple>

namespace std {
	template <class...>
	class tuple;
}

namespace templates_detail {
	template <
		class T,
		unsigned Index
	>
	struct found_type_result {
		typedef T type;
		static constexpr bool found = true;
		static constexpr unsigned index = Index;
	};
}

template <
	unsigned CurrentCandidateIndex,
	template <class, class> class Criterion,
	class SearchedType,
	class List
>
struct find_matching_type_detail;

template <
	unsigned CurrentCandidateIndex,
	template<class, class> class Criterion,
	class SearchedType,
	template <class...> class List
>
struct find_matching_type_detail<
	CurrentCandidateIndex,
	Criterion,
	SearchedType,
	List<>
> {
	static constexpr bool found = false;
};

template <
	unsigned CurrentCandidateIndex,
	template<class, class> class Criterion,
	class SearchedType,
	template <class...> class List,
	class Candidate,
	class... Candidates
>
struct find_matching_type_detail<
	CurrentCandidateIndex,
	Criterion,
	SearchedType,
	List<Candidate, Candidates...>
> :
	std::conditional_t<
		Criterion<SearchedType, Candidate>::value, 
		templates_detail::found_type_result<Candidate, CurrentCandidateIndex>,
		find_matching_type_detail<
			CurrentCandidateIndex + 1,
			Criterion, 
			SearchedType, 
			List<Candidates...>
		>
	> 
{
};

template <
	template<class, class> class Criterion,
	class SearchedType,
	class List
>
using find_matching_type = find_matching_type_detail<0, Criterion, SearchedType, List>;

template <class S, class List>
using is_one_of_list = std::bool_constant<find_matching_type<std::is_same, S, List>::found>;

template <class S, class... Types>
using is_one_of = is_one_of_list<S, std::tuple<Types...>>;

template <class S, class List>
constexpr bool is_one_of_list_v = find_matching_type<std::is_same, S, List>::found;

template <class S, class... Types>
constexpr bool is_one_of_v = is_one_of_list_v<S, std::tuple<Types...>>;

template <class S, class List>
constexpr unsigned index_in_list_v = find_matching_type<std::is_same, S, List>::index;

template <class S, class... Types>
constexpr unsigned index_in_v = index_in_list_v<S, std::tuple<Types...>>;

template <class S, class List>
using find_convertible_type_in_list_t = typename find_matching_type<std::is_convertible, S, List>::type;

template <class S, class... Types>
using find_convertible_type_in_t = find_convertible_type_in_list_t<S, std::tuple<Types...>>;

template <unsigned idx, class... Types>
struct nth_type_in {
	static_assert(idx < sizeof...(Types), "Type index out of bounds!");
	typedef std::decay_t<decltype(std::get<idx>(std::tuple<Types...>()))> type;
};

template<unsigned idx, class... Types>
using nth_type_in_t = typename nth_type_in<idx, Types...>::type;

static_assert(is_one_of_list_v<unsigned, std::tuple<float, float, double, unsigned>>, "Something wrong with trait");
static_assert(is_one_of_v<unsigned, float, float, double, unsigned>, "Something wrong with trait");

static_assert(index_in_list_v<unsigned, std::tuple<float, float, double, unsigned>> == 3, "Something wrong with trait");
static_assert(index_in_v<unsigned, float, float, double, unsigned> == 3, "Something wrong with trait");

static_assert(std::is_same_v<unsigned, nth_type_in_t<0, unsigned, float, float>>, "Something wrong with trait");
static_assert(std::is_same_v<double, nth_type_in_t<3, unsigned, float, float, double, unsigned>>, "Something wrong with trait");

template <class T, class Candidate>
struct is_key_type_equal_to : std::bool_constant<std::is_same_v<T, typename Candidate::key_type>> {

};

template <class SearchedKeyType, class List>
using find_type_with_key_type_in_list_t = typename find_matching_type<is_key_type_equal_to, SearchedKeyType, List>::type;

template <class SearchedKeyType, class... Types>
using find_type_with_key_type_t = find_type_with_key_type_in_list_t<SearchedKeyType, std::tuple<Types...>>;
