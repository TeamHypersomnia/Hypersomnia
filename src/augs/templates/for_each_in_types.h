#pragma once
#include <type_traits>
#include <tuple>

#include "augs/templates/transform_types.h"

namespace templates_detail {
	template <class F, class... Instances>
	void for_each_through_std_get(F&& f, std::index_sequence<>, Instances&&... instances)
	{}

	template <class F, class... Instances, size_t N, size_t... Is>
	void for_each_through_std_get(F&& f, std::index_sequence<N, Is...>, Instances&&... instances) {
		f(N, std::get<N>(std::forward<Instances>(instances))...);
		
		for_each_through_std_get(
			std::forward<F>(f), 
			std::index_sequence<Is...>(), 
			std::forward<Instances>(instances)...
		);
	}
}

template <class>
struct index_sequence_for_list;

template <template <class...> class List, class... Args>
struct index_sequence_for_list<List<Args...>> {
	typedef std::index_sequence_for<Args...> type;
};

template <class T>
using index_sequence_for_list_t = typename index_sequence_for_list<T>::type;

template <
	class List,
	class F
>
void for_each_through_std_get(List&& t, F f) {
	templates_detail::for_each_through_std_get(
		[f](auto num, auto&&... args) {
			f(std::forward<decltype(args)>(args)...);
		},
		index_sequence_for_list_t<std::decay_t<List>>{},
		std::forward<List>(t)
	);
}


template <class F>
void for_each_type(F&&) {}

template <class T, class... Ts, class F>
void for_each_type(F&& f) {
	f(T());
	for_each_type<Ts...>(std::forward<F>(f));
}

template <class L, class F>
void for_each_type_in_list(F&& f) {
	for_each_through_std_get(
		replace_list_type_t<augs::paint_command_variant, std::tuple>(), 
		std::forward<F>(f)
	);
}
