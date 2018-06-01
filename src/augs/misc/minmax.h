#pragma once
#include "augs/misc/simple_pair.h"

namespace augs {
	template <class T>
	using minmax = simple_pair<T, T>;

	template<class T>
	using random_bound = simple_pair<minmax<T>, minmax<T>>;
}

template <class T>
struct is_minmax : std::false_type {};

template <class T>
struct is_minmax<augs::minmax<T>> : std::true_type {};

template <class T>
constexpr bool is_minmax_v = is_minmax<T>::value;


template <class T>
struct is_arithmetic_minmax : std::false_type {};

template <class T>
struct is_arithmetic_minmax<augs::minmax<T>> : std::bool_constant<std::is_arithmetic_v<T>> {};

template <class T>
constexpr bool is_arithmetic_minmax_v = is_arithmetic_minmax<T>::value;
