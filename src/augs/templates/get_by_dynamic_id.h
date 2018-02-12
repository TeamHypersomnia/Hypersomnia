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
decltype(auto) get_by_dynamic_index(
	T&& index_gettable_object,
	const std::size_t dynamic_type_index,
	F&& generic_call
) {
	if constexpr(current_candidate < num_types_in_list_v<std::decay_t<T>>) {
		if (current_candidate == dynamic_type_index) {
			return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
		}

		return get_by_dynamic_index<current_candidate + 1>(
			std::forward<T>(index_gettable_object),
			dynamic_type_index,
			std::forward<F>(generic_call)
		);
	}
	else {
		LOG_NVPS(dynamic_type_index);
		ensure(false && "dynamic_type_index is out of bounds!");
		return generic_call(std::get<0>(std::forward<T>(index_gettable_object)));
	}
}

template <
	class OnlyCandidates,
	std::size_t current_candidate = 0,
	class T,
	class F
>
decltype(auto) conditional_get_by_dynamic_index(
	T&& index_gettable_object,
	const std::size_t dynamic_type_index,
	F&& generic_call
) {
	using list_type = std::decay_t<T>;

	if constexpr(current_candidate < num_types_in_list_v<list_type>) {
		if constexpr(
			is_one_of_list_v<
				decltype(std::get<current_candidate>(std::forward<T>(index_gettable_object))),
				OnlyCandidates
			>
		) {
			if (current_candidate == dynamic_type_index) {
				return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
			}
		}

		return conditional_get_by_dynamic_index<OnlyCandidates, current_candidate + 1>(
			std::forward<T>(index_gettable_object),
			dynamic_type_index,
			std::forward<F>(generic_call)
		);
	}
	else {
		LOG_NVPS(dynamic_type_index);
		ensure(false && "dynamic_type_index is out of bounds!");
		return generic_call(std::get<
			index_in_list_v<
				nth_type_in_list_t<0, OnlyCandidates>,
				list_type
			>
		>(std::forward<T>(index_gettable_object)));
	}
}

template <class T, class F>
decltype(auto) get_by_dynamic_id(
	T&& index_gettable_object,
	const type_in_list_id<std::decay_t<T>> dynamic_type_index,
	F&& generic_call
) {
	return get_by_dynamic_index<0>(
		std::forward<T>(index_gettable_object), 
		static_cast<std::size_t>(dynamic_type_index.get_index()),
		std::forward<F>(generic_call)
	);
}

template <class OnlyCandidates, class T, class F>
decltype(auto) conditional_get_by_dynamic_id(
	T&& index_gettable_object,
	const type_in_list_id<std::decay_t<T>> dynamic_type_index,
	F&& generic_call
) {
	return conditional_get_by_dynamic_index<OnlyCandidates, 0>(
		std::forward<T>(index_gettable_object), 
		static_cast<std::size_t>(dynamic_type_index.get_index()),
		std::forward<F>(generic_call)
	);
}