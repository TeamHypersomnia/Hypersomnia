#pragma once
#include <xtr1common>

template <class...>
struct type_list {

};

/*
	Applicable to any kind of list, not just struct type_list,
	tuples for example.
*/

template <class T>
struct num_types_in_list {
	static constexpr std::size_t value = 0;
};

template <template <class...> class T, class... Types>
struct num_types_in_list<T<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <class T>
constexpr std::size_t num_types_in_list_v = num_types_in_list<T>::value;