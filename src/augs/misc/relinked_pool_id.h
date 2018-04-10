#pragma once
#include <ostream>
#include <type_traits>
#include <limits>

#include "augs/templates/hash_fwd.h"

template <class T>
struct relinked_pool_id {
	// GEN INTROSPECTOR struct relinked_pool_id class T
	T value;
	// END GEN INTROSPECTOR

	static_assert(std::is_pod_v<T>, "The type is not POD!");

	static relinked_pool_id INVALID;

	relinked_pool_id(const T p = static_cast<T>(-1)) : value(p) {}

	bool operator==(const T b) const {
		return value == b;
	}

	bool operator!=(const T b) const {
		return value != b;
	}

	bool operator==(const relinked_pool_id<T> b) const {
		return value == b.value;
	}

	bool operator!=(const relinked_pool_id<T> b) const {
		return value != b.value;
	}

	operator T() const {
		return value;
	}

	explicit operator bool() const {
		return value < std::numeric_limits<T>::max() / 2;
	}
};

template <class T>
relinked_pool_id<T> relinked_pool_id<T>::INVALID = {};

template <class T>
std::ostream& operator<<(std::ostream& out, const relinked_pool_id<T>& x) {
	return out << x.value;
}

namespace std {
	template <class T>
	struct hash<relinked_pool_id<T>> {
		std::size_t operator()(const relinked_pool_id<T>& z) const {
			return std::hash<T>()(z.value);
		}
	};
}