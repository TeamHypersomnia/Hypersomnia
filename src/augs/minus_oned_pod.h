#pragma once
#include <ostream>
#include <type_traits>
#include "augs/templates/hash_fwd.h"

template <class T>
struct minus_oned_pod {
	// GEN INTROSPECTOR struct minus_oned_pod class T
	T value;
	// END GEN INTROSPECTOR

	static_assert(std::is_pod_v<T>, "The type is not POD!");

	static minus_oned_pod INVALID;

	minus_oned_pod(const T p = static_cast<T>(-1)) : value(p) {}

	bool operator==(const T b) const {
		return value == b;
	}

	bool operator!=(const T b) const {
		return value != b;
	}

	bool operator==(const minus_oned_pod<T> b) const {
		return value == b.value;
	}

	bool operator!=(const minus_oned_pod<T> b) const {
		return value != b.value;
	}

	operator T() const {
		return value;
	}

	explicit operator bool() const {
		return value != -1;
	}
};

template <class T>
minus_oned_pod<T> minus_oned_pod<T>::INVALID = {};

template <class T>
std::ostream& operator<<(std::ostream& out, const minus_oned_pod<T>& x) {
	return out << x.value;
}

namespace std {
	template <class T>
	struct hash<minus_oned_pod<T>> {
		std::size_t operator()(const minus_oned_pod<T>& z) const {
			return std::hash<T>()(z.value);
		}
	};
}