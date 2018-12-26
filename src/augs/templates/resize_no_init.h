#pragma once
#include <vector>
#include <type_traits>
#include "augs/misc/declare_containers.h"

template <class V>
void resize_no_init(std::vector<V>& v, const std::size_t n) {
	if constexpr(std::is_trivial_v<V>) {
		/* For now, no safe implementation is available. */
		v.resize(n);
	}
	else {
		v.resize(n);
	}
}

template <class V, unsigned const_count>
void resize_no_init(augs::constant_size_vector<V, const_count>& v, const std::size_t n) {
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

inline void resize_no_init(std::string& v, const std::size_t n) {
	v.resize(n);
}
