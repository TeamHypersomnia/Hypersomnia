#pragma once
#include <type_traits>

template <class F>
void for_each_type(F&&) {}

template <class T, class... Ts, class F>
void for_each_type(F&& f) {
	f(T());
	for_each_type<Ts...>(std::forward<F>(f));
}

template <class T>
struct for_each_type_in_list_impl;

template <template <class...> class List, class... Args>
struct for_each_type_in_list_impl<List<Args...>> {
	template <class F>
	void operator()(F&& f) const {
		for_each_type<Args...>(std::forward<F>(f));
	}
};

template <class L, class F>
void for_each_type_in_list(F&& f) {
	for_each_type_in_list_impl<L>()(std::forward<F>(f));
}
