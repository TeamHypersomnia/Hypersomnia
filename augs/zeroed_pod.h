#pragma once
#include <type_traits>

template <class T>
class zeroed_pod {
	T pod;
public:

	static_assert(std::is_pod<T>::value, "type is not POD!");
	
	zeroed_pod(const T p = static_cast<T>(0)) : pod(p) {}

	operator T() const {
		return pod;
	}
};