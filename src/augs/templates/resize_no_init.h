#pragma once
#include <cstddef>
#include <type_traits>
#include "augs/misc/declare_containers.h"

template <class V, unsigned const_count, bool F>
void resize_no_init(augs::constant_size_vector<V, const_count, F>& v, const std::size_t n) {
	if constexpr(std::is_trivial_v<V>) {
		v.resize_no_init(n);
	}
	else {
		v.resize(n);
	}
}

template <unsigned const_count>
void resize_no_init(augs::constant_size_string<const_count>& v, const std::size_t n) {
	v.resize_no_init(n);
}

template <class T>
void resize_no_init(T& v, const std::size_t n) {
	/* For now, no safe implementation for vectors and the like is available. */
	v.resize(n);
}
