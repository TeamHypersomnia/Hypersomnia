#pragma once
#include "augs/ensure.h"
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/type_list.h"

template <
	std::size_t current_candidate = 0,
	class T,
	class F
>
decltype(auto) visit_list(
	T&& index_gettable_object,
	const type_in_list_id<std::decay_t<T>> dynamic_type_index,
	F&& generic_call,
	std::enable_if_t<current_candidate >= num_types_in_list_v<std::decay_t<T>>>* dummy = nullptr
) {
	LOG_NVPS(dynamic_type_index.get_index());
	ensure(false && "dynamic_type_index is out of bounds!");
	return generic_call(std::get<0>(index_gettable_object));
}

template <
	std::size_t current_candidate = 0,
	class T,
	class F
>
decltype(auto) visit_list(
	T&& index_gettable_object,
	const type_in_list_id<std::decay_t<T>> dynamic_type_index,
	F&& generic_call,
	std::enable_if_t<current_candidate < num_types_in_list_v<std::decay_t<T>>>* dummy = nullptr
) {
	if (current_candidate == dynamic_type_index.get_index()) {
		return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
	}

	return visit_list<current_candidate + 1, T, F>(
		index_gettable_object,
		dynamic_type_index,
		std::forward<F>(generic_call)
	);
}