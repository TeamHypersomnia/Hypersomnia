#pragma once
#include <type_traits>
#include "augs/templates/hash_fwd.h"

template <class T>
struct zeroed_pod {
	// GEN INTROSPECTOR struct zeroed_pod class T
	T pod;
	// END GEN INTROSPECTOR

	static_assert(std::is_pod_v<T>, "type is not POD!");
	
	zeroed_pod(const T p = static_cast<T>(0)) : pod(p) {}

	bool operator==(const T b) const {
		return pod == b;
	}

	bool operator!=(const T b) const {
		return pod != b;
	}

	bool operator==(const zeroed_pod<T> b) const {
		return pod == b.pod;
	}

	bool operator!=(const zeroed_pod<T> b) const {
		return pod != b.pod;
	}

	operator T() const {
		return pod;
	}
};

template <class T>
struct minus_oned_pod {
	// GEN INTROSPECTOR struct minus_oned_pod class T
	T pod;
	// END GEN INTROSPECTOR

	static_assert(std::is_pod_v<T>, "type is not POD!");

	minus_oned_pod(const T p = static_cast<T>(-1)) : pod(p) {}

	bool operator==(const T b) const {
		return pod == b;
	}

	bool operator!=(const T b) const {
		return pod != b;
	}

	bool operator==(const minus_oned_pod<T> b) const {
		return pod == b.pod;
	}

	bool operator!=(const minus_oned_pod<T> b) const {
		return pod != b.pod;
	}

	operator T() const {
		return pod;
	}

	operator bool() const {
		return pod != -1;
	}
};

namespace std {
	template <class T>
	struct hash<zeroed_pod<T>> {
		std::size_t operator()(const zeroed_pod<T>& z) const {
			return std::hash<T>()(T(z));
		}
	};

	template <class T>
	struct hash<minus_oned_pod<T>> {
		std::size_t operator()(const minus_oned_pod<T>& z) const {
			return std::hash<T>()(T(z));
		}
	};
}

template <class T>
struct zeroed_pod_internal_type {
	typedef T type;
};

template <class T>
struct zeroed_pod_internal_type<zeroed_pod<T>> {
	typedef T type;
};

template <class T>
using zeroed_pod_internal_type_t = typename zeroed_pod_internal_type<T>::type;