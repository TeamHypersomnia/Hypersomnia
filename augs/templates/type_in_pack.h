#pragma once
#include <tuple>
#include "minimal_templates.h"

template <typename T, typename Tuple>
struct has_type;

template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template <typename T, typename Tuple>
using tuple_contains_type = typename has_type<T, Tuple>::type;

template <typename T, typename... Types>
using pack_contains_type = typename has_type<T, std::tuple<Types...>>::type;

template<unsigned idx, class... Types>
struct nth_type_in {
	static_assert(idx < sizeof...(Types), "Type index out of bounds!");
	typedef std::decay_t<decltype(std::get<idx>(std::tuple<Types...>()))> type;
};

template<unsigned idx, class... Types>
using nth_type_in_t = typename nth_type_in<idx, Types...>::type;

template <class T, class Tuple>
struct index_in_tuple;

template <class T, class... Types>
struct index_in_tuple<T, std::tuple<T, Types...>> {
	static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct index_in_tuple<T, std::tuple<U, Types...>> {
	static_assert(tuple_contains_type<T, std::tuple<U, Types...>>::value, "No such type in the tuple or parameter pack!");

	static const std::size_t value = 1 + index_in_tuple<T, std::tuple<Types...>>::value;
};

template<class T, class... Types>
struct index_in_pack {
	static const size_t value = index_in_tuple<T, std::tuple<Types...>>::value;
};
