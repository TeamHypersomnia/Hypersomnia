#pragma once
#include <type_traits>
#include <tuple>
#include <variant>
#include "augs/misc/trivially_copyable_tuple.h"

namespace templates_detail {
	template <class F, class... Instances>
	void for_each_through_std_get(F&& f, std::index_sequence<>, Instances&&... instances)
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

	template <class>
	struct index_sequence_for_list;

	template <template <class...> class List, class... Args>
	struct index_sequence_for_list<List<Args...>> {
		using type = std::index_sequence_for<Args...>;
	};

	template <class T>
	using index_sequence_for_list_t = typename index_sequence_for_list<T>::type;
}

template <class List, class F>
void for_each_through_std_get(List&& t, F f) {
	using namespace templates_detail;

	for_each_through_std_get(
		[f](auto num, auto&&... args) {
			f(std::forward<decltype(args)>(args)...);
		},
		templates_detail::index_sequence_for_list_t<std::decay_t<List>>{},
		std::forward<List>(t)
	);
}

template <class List, class F>
void reverse_for_each_through_std_get(List&& t, F f) {
	using namespace templates_detail;

	for_each_through_std_get(
		[f](auto num, auto&&... args) {
			f(std::forward<decltype(args)>(args)...);
		},
		reverse_sequence_t<templates_detail::index_sequence_for_list_t<std::decay_t<List>>>{},
		std::forward<List>(t)
	);
}