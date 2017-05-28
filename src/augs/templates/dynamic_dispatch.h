#pragma once
#include "augs/ensure.h"

template <class T>
struct num_types_in_list {
	static constexpr std::size_t value = 0;
};

template <template <class...> class T, class... Types>
struct num_types_in_list<T<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <class T>
constexpr std::size_t num_types_in_list_v = num_types_in_list<T>::value;

template <
	std::size_t current_candidate = 0,
	class T,
	class F
>
decltype(auto) dynamic_dispatch(
	T&& index_gettable_object,
	const std::size_t dynamic_type_index,
	F&& generic_call,
	std::enable_if_t<current_candidate >= num_types_in_list_v<std::decay_t<T>>>* dummy = nullptr
) {
	LOG_NVPS(dynamic_type_index);
	ensure(false && "dynamic_type_index is out of bounds!");
	return generic_call(std::get<0>(index_gettable_object));
}

template <
	std::size_t current_candidate = 0,
	class T,
	class F
>
decltype(auto) dynamic_dispatch(
	T&& index_gettable_object,
	const std::size_t dynamic_type_index,
	F&& generic_call,
	std::enable_if_t<current_candidate < num_types_in_list_v<std::decay_t<T>>>* dummy = nullptr
) {
	if (current_candidate == dynamic_type_index) {
		return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
	}

	return dynamic_dispatch<current_candidate + 1, T, F>(
		index_gettable_object,
		dynamic_type_index,
		std::forward<F>(generic_call)
	);
}