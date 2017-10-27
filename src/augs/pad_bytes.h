#pragma once
#include <array>
#include "augs/zeroed_pod.h"

template <std::size_t count>
struct pad_bytes {
	// GEN INTROSPECTOR struct pad_bytes std::size_t count
	std::array<zeroed_pod<char>, count> pad;
	// END GEN INTROSPECTOR
};

template <class T>
struct is_padding_field : std::false_type {

};

template <std::size_t I>
struct is_padding_field<pad_bytes<I>> : std::true_type {

};

template <class T>
constexpr bool is_padding_field_v = is_padding_field<T>::value;