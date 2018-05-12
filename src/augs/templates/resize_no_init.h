#pragma once
#include <vector>
#include <type_traits>
#include "augs/misc/declare_containers.h"

template <class V>
void resize_no_init(std::vector<V>& v, const std::size_t n) {
	if constexpr(std::is_trivial_v<V>) {
		/* 
			Only trivial types require this treatment.
			Otherwise, it is the responsibility of the class to introduce
			a non-initializing constructor.
		*/

		struct no_init {
			V v;
			no_init() {}
		};

		static_assert(sizeof(no_init) == sizeof(V));
		static_assert(alignof(no_init) == alignof(V));

		reinterpret_cast<std::vector<no_init>&>(v).resize(n);
	}
	else {
		v.resize(n);
	}
}

template <class V, unsigned const_count>
void resize_no_init(augs::constant_size_vector<V, const_count>& v, const std::size_t n) {
	if constexpr(std::is_trivial_v<V>) {
		/* 
			Only trivial types require this treatment.
			Otherwise, it is the responsibility of the class to introduce
			a non-initializing constructor.
		*/

		struct no_init {
			V v;
			no_init() {}
		};

		static_assert(sizeof(no_init) == sizeof(V));
		static_assert(alignof(no_init) == alignof(V));

		reinterpret_cast<augs::constant_size_vector<no_init, const_count>&>(v).resize(n);
	}
	else {
		v.resize(n);
	}
}

inline void resize_no_init(std::string& v, const std::size_t n) {
	v.resize(n);
}
