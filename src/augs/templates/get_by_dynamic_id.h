#pragma once
#include <tuple>
#include <variant>
#include "augs/misc/trivially_copyable_tuple.h"

#include "augs/ensure.h"
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/type_list.h"

template <
	std::size_t current_candidate = 0,
	class T,
	class F
>
decltype(auto) get_by_dynamic_id(
	T&& index_gettable_object,
	const std::size_t dynamic_type_index,
	F&& generic_call
) {
	if constexpr(current_candidate < num_types_in_list_v<std::decay_t<T>>) {
		if (current_candidate == dynamic_type_index) {
			return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
		}

		return get_by_dynamic_id<current_candidate + 1, T, F>(
			index_gettable_object,
			dynamic_type_index,
			std::forward<F>(generic_call)
		);
	}
	else {
		LOG_NVPS(dynamic_type_index);
		ensure(false && "dynamic_type_index is out of bounds!");
		return generic_call(std::get<0>(index_gettable_object));
	}
}

template <
	class OnlyCandidates,
	std::size_t current_candidate = 0,
	class T,
	class F
>
decltype(auto) get_by_dynamic_id(
	T&& index_gettable_object,
	const std::size_t dynamic_type_index,
	F&& generic_call
) {
	using list_type = std::decay_t<T>;

	if constexpr(current_candidate < num_types_in_list_v<list_type>) {
		if constexpr(
			is_one_of_list_v<
				nth_type_in_list_t<current_candidate, list_type>,
				OnlyCandidates
			>
		) {
			if (current_candidate == dynamic_type_index) {
				return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
			}
		}

		return get_by_dynamic_id<OnlyCandidates, current_candidate + 1, T, F>(
			index_gettable_object,
			dynamic_type_index,
			std::forward<F>(generic_call)
		);
	}
	else {
		LOG_NVPS(dynamic_type_index);
		ensure(false && "dynamic_type_index is out of bounds!");
		return generic_call(std::get<0>(index_gettable_object));
	}
}

template <class T, class F>
decltype(auto) get_by_dynamic_id(
	T&& index_gettable_object,
	const type_in_list_id<std::decay_t<T>> dynamic_type_index,
	F&& generic_call
) {
	return get_by_dynamic_id(
		std::forward<T>(index_gettable_object), 
		static_cast<std::size_t>(dynamic_type_index.get_index()),
		std::forward<F>(generic_call)
	);
}

template <class OnlyCandidates, class T, class F>
decltype(auto) get_by_dynamic_id(
	T&& index_gettable_object,
	const type_in_list_id<std::decay_t<T>> dynamic_type_index,
	F&& generic_call
) {
	return get_by_dynamic_id<OnlyCandidates>(
		std::forward<T>(index_gettable_object), 
		static_cast<std::size_t>(dynamic_type_index.get_index()),
		std::forward<F>(generic_call)
	);
}