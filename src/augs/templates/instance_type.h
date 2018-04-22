#pragma once
#include <type_traits>

template <class T>
using instance_of = typename T::instance;

template <class T, class MetaTuple>
decltype(auto) get_meta_of(T&&, MetaTuple&& t) {
	return std::get<typename std::decay_t<T>::meta_type>(std::forward<MetaTuple>(t));
}