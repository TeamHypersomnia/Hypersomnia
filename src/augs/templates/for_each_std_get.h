#pragma once
#include <cstddef>
#include <type_traits>
#include <tuple>
#include <variant>

#include "augs/templates/sequence_utils.h"
#include "augs/misc/trivially_copyable_tuple.h"
#include "augs/templates/type_list.h"
#include "augs/templates/remove_cref.h"

namespace templates_detail {
	template <class F, class... Instances>
	void for_each_through_std_get(F&&, std::index_sequence<>, Instances&&...)
	{}

	template <class F, class... Instances, std::size_t N, std::size_t... Is>
	void for_each_through_std_get(F&& f, std::index_sequence<N, Is...>, Instances&&... instances) {
		f(N, std::get<N>(std::forward<Instances>(instances))...);
		
		for_each_through_std_get(
			std::forward<F>(f), 
			std::index_sequence<Is...>(), 
			std::forward<Instances>(instances)...
		);
	}
}

template <class List, class F>
void for_each_through_std_get(List&& t, F f) {
	using namespace templates_detail;

	for_each_through_std_get(
		[f](auto, auto&&... args) {
			f(std::forward<decltype(args)>(args)...);
		},
		std::make_index_sequence<num_types_in_list_v<remove_cref<List>>>(),
		std::forward<List>(t)
	);
}

template <class List, class F>
void reverse_for_each_through_std_get(List&& t, F f) {
	using namespace templates_detail;

	for_each_through_std_get(
		[f](auto, auto&&... args) {
			f(std::forward<decltype(args)>(args)...);
		},
		reverse_sequence_t<std::make_index_sequence<num_types_in_list_v<remove_cref<List>>>>(),
		std::forward<List>(t)
	);
}