#pragma once

template <class A, class B>
struct type_pair {
	using First = A;
	using Second = B;
};

template <class A, class T, T val>
struct type_value_pair {
	using First = A;

	static constexpr T Value = val;
};

template <class A, uint32_t val>
using type_uint32_pair = type_value_pair<A, uint32_t, val>;
