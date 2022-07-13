#pragma once
#include <type_traits>
#include "augs/templates/always_false.h"

template <class...>
struct type_list;

template <class S, class... Types>
constexpr bool is_derived_from_any_of_v = (... || std::is_base_of_v<Types, S>);

template <class S, class... Types>
constexpr bool is_one_of_v = (... || std::is_same_v<S, Types>);

template <template <class> class Criterion, class... Types>
constexpr bool all_are_v = (... && Criterion<Types>::value);


template <class A, class T>
struct is_one_of_list;

template <class A, template <class...> class L, class... Types>
struct is_one_of_list<A, L<Types...>> : std::bool_constant<is_one_of_v<A, Types...>> {};


template <template <class> class Criterion, class T>
struct all_in_list_are;

template <template <class> class Criterion, template <class...> class L, class... Types>
struct all_in_list_are<Criterion, L<Types...>> : std::bool_constant<all_are_v<Criterion, Types...>> {};


template <class S, class List>
constexpr bool is_one_of_list_v = is_one_of_list<S, List>::value;

template <template <class> class Criterion, class List>
constexpr bool all_in_list_are_v = all_in_list_are<Criterion, List>::value;


template <std::size_t i, class Candidate, class List>
struct index_in_list;

template <std::size_t i, class Candidate, template <class...> class List>
struct index_in_list<i, Candidate, List<>> {
	static_assert(always_false_v<Candidate>, "Type was not found via index_in_list.");
};

template <std::size_t i, class Candidate, template <class...> class List, class T, class... Types>
struct index_in_list<i, Candidate, List<T, Types...>> {
	static constexpr std::size_t value = index_in_list<i + 1, Candidate, List<Types...>>::value;
};

template <std::size_t i, class Candidate, template <class...> class List, class... Types>
struct index_in_list<i, Candidate, List<Candidate, Types...>> {
	static constexpr std::size_t value = i;
};

template <class S, class List>
constexpr std::size_t index_in_list_v = index_in_list<0, S, List>::value;

template <class S, class... Types>
constexpr std::size_t index_in_v = index_in_list_v<S, type_list<Types...>>;


template <std::size_t... indices>
constexpr std::size_t folded_sum_v = (... + indices);
