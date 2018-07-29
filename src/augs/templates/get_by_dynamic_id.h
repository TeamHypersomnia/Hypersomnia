#pragma once
#include <tuple>
#include <variant>
#include "augs/misc/trivially_copyable_tuple.h"

#include "augs/ensure.h"
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/type_list.h"
#include "augs/templates/nth_type_in.h"

template <
	std::size_t current_candidate = 0,
	bool nullopt_if_not_found = false,
	class T,
	class F
>
decltype(auto) get_by_dynamic_index(
	T&& index_gettable_object,
	const std::size_t dynamic_type_index,
	F&& generic_call
) {
	static constexpr std::size_t last_candidate = num_types_in_list_v<remove_cref<T>> - 1;
	static constexpr bool is_last = current_candidate == last_candidate;

	if constexpr(is_last) {
		if constexpr(nullopt_if_not_found) {
			if (current_candidate == dynamic_type_index) {
				return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
			}
			else {
				return generic_call(std::nullopt);
			}
		}
		else {
			ensure_eq(dynamic_type_index, current_candidate);
			return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
		}
	}
	else {
		if (current_candidate == dynamic_type_index) {
			return generic_call(std::get<current_candidate>(std::forward<T>(index_gettable_object)));
		}

		return get_by_dynamic_index<current_candidate + 1, nullopt_if_not_found>(
			std::forward<T>(index_gettable_object),
			dynamic_type_index,
			std::forward<F>(generic_call)
		);
	}
}

template <
	class OnlyCandidates,
	std::size_t candidate_space_current = 0,
	bool nullopt_if_not_found = false,
	class T,
	class F
>
decltype(auto) conditional_get_by_dynamic_index(
	T&& index_gettable_object,
	const std::size_t dynamic_type_index,
	F&& generic_call
) {
	using list_type = remove_cref<T>;

	static constexpr std::size_t last_candidate = num_types_in_list_v<OnlyCandidates> - 1;
	static constexpr bool is_last = candidate_space_current == last_candidate;
	static constexpr std::size_t list_space_current = 
		index_in_list_v<
			nth_type_in_list_t<candidate_space_current, OnlyCandidates>,
			list_type
		>
	;

	if constexpr(is_last) {
		if constexpr(nullopt_if_not_found) {
			if (list_space_current == dynamic_type_index) {
				return generic_call(std::get<list_space_current>(std::forward<T>(index_gettable_object)));
			}
			else {
				return generic_call(std::nullopt);
			}
		}
		else {
			ensure_eq(dynamic_type_index, list_space_current);
			return generic_call(std::get<list_space_current>(std::forward<T>(index_gettable_object)));
		}
	}
	else {
		if (list_space_current == dynamic_type_index) {
			return generic_call(std::get<list_space_current>(std::forward<T>(index_gettable_object)));
		}

		return conditional_get_by_dynamic_index<OnlyCandidates, candidate_space_current + 1, nullopt_if_not_found>(
			std::forward<T>(index_gettable_object),
			dynamic_type_index,
			std::forward<F>(generic_call)
		);
	}
}

template <class T, class F>
decltype(auto) get_by_dynamic_id(
	T&& index_gettable_object,
	const type_in_list_id<remove_cref<T>> dynamic_type_index,
	F&& generic_call
) {
	static_assert(num_types_in_list_v<remove_cref<T>> > 0, "Can't get from an empty list.");

	return get_by_dynamic_index<0>(
		std::forward<T>(index_gettable_object), 
		static_cast<std::size_t>(dynamic_type_index.get_index()),
		std::forward<F>(generic_call)
	);
}

template <class OnlyCandidates, class T, class F>
decltype(auto) conditional_get_by_dynamic_id(
	T&& index_gettable_object,
	const type_in_list_id<remove_cref<T>> dynamic_type_index,
	F&& generic_call
) {
	static_assert(num_types_in_list_v<remove_cref<T>> > 0, "Can't get from an empty list.");
	static_assert(num_types_in_list_v<OnlyCandidates> > 0, "Candidate list is empty.");

	return conditional_get_by_dynamic_index<OnlyCandidates, 0>(
		std::forward<T>(index_gettable_object), 
		static_cast<std::size_t>(dynamic_type_index.get_index()),
		std::forward<F>(generic_call)
	);
}

template <class T, class F>
decltype(auto) find_by_dynamic_id(
	T&& index_findable_object,
	const type_in_list_id<remove_cref<T>> dynamic_type_index,
	F&& generic_call
) {
	static_assert(num_types_in_list_v<remove_cref<T>> > 0, "Can't find from an empty list.");

	return get_by_dynamic_index<0, true>(
		std::forward<T>(index_findable_object), 
		static_cast<std::size_t>(dynamic_type_index.get_index()),
		std::forward<F>(generic_call)
	);
}

template <class OnlyCandidates, class T, class F>
decltype(auto) conditional_find_by_dynamic_id(
	T&& index_findable_object,
	const type_in_list_id<remove_cref<T>> dynamic_type_index,
	F&& generic_call
) {
	static_assert(num_types_in_list_v<remove_cref<T>> > 0, "Can't find from an empty list.");

	if constexpr(std::is_same_v<OnlyCandidates, type_list<>>) {
		return generic_call(std::nullopt);
	}
	else {
		return conditional_get_by_dynamic_index<OnlyCandidates, 0, true>(
			std::forward<T>(index_findable_object), 
			static_cast<std::size_t>(dynamic_type_index.get_index()),
			std::forward<F>(generic_call)
		);
	}
}