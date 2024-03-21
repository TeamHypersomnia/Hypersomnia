#pragma once
#include <cstddef>

template <class...>
struct type_list;

template <std::size_t I, class ListType>
struct nth_type_in {};

template <template <class...> class List, class Candidate, class... Types>
struct nth_type_in<0, List<Candidate, Types...>> {
	using type = Candidate;
};

template <std::size_t I, template <class...> class List, class Candidate, class... Types>
struct nth_type_in<I, List<Candidate, Types...>> {
	using type = typename nth_type_in<I - 1, List<Types...>>::type;
};

template <size_t I, class... Types>
using nth_type_in_t = typename nth_type_in<I, type_list<Types...>>::type;

template <size_t I, class ListType>
using nth_type_in_list_t = typename nth_type_in<I, ListType>::type;

template <class T, class... Types>
struct first_type_getter {
	using type = T;
};

template <class... Types>
using first_type_t = typename first_type_getter<Types...>::type;