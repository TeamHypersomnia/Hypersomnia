#pragma once
#include <type_traits>
#include <cstddef>

#include "augs/templates/nth_type_in.h"

template <class...>
struct type_list {};

template <class T>
struct is_type_list : std::false_type {};

template <class... Types>
struct is_type_list<type_list<Types...>> : std::true_type {};

template <class T>
static constexpr bool is_type_list_v = is_type_list<T>::value;

namespace std {
	template <std::size_t I, class... Types>
	const auto& get(const type_list<Types...>&) {
		using type = nth_type_in_t<I, Types...>;

		static const type instance = {};
		return instance;
	}
}

/*
	Applicable to any kind of list, not just struct type_list,
	tuples for example.
*/

template <class T>
struct num_types_in_list;

template <template <class...> class T, class... Types>
struct num_types_in_list<T<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <class T>
constexpr std::size_t num_types_in_list_v = num_types_in_list<T>::value;