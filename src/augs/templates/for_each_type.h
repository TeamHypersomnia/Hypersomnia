#pragma once
#include <type_traits>

template <class T>
struct for_each_type_in_list_impl;

template <template <class...> class List, class... Args>
struct for_each_type_in_list_impl<List<Args...>> {
	template <class F>
	void operator()(F&& f) const {
		(f(Args()), ...);
	}
};

template <class L, class F>
void for_each_type_in_list(F&& f) {
	for_each_type_in_list_impl<L>()(std::forward<F>(f));
}
