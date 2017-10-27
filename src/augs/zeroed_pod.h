#pragma once
#include <type_traits>

template <class T>
struct zeroed_pod {
	// GEN INTROSPECTOR struct zeroed_pod class T
	T pod;
	// END GEN INTROSPECTOR

	static_assert(std::is_pod_v<T>, "type is not POD!");
	
	zeroed_pod(const T p = static_cast<T>(0)) : pod(p) {}

	operator T() const {
		return pod;
	}
};


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