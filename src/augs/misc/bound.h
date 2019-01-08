#pragma once
#include "augs/misc/simple_pair.h"

namespace augs {
	template <class T>
	using bound = simple_pair<T, T>;

	template<class T>
	using random_bound = simple_pair<bound<T>, bound<T>>;
}

template <class T>
struct is_bound : std::false_type {};

template <class T>
struct is_bound<augs::bound<T>> : std::true_type {};

template <class T>
constexpr bool is_bound_v = is_bound<T>::value;


template <class T>
struct is_arithmetic_bound : std::false_type {};

template <class T>
struct is_arithmetic_bound<augs::bound<T>> : std::bool_constant<std::is_arithmetic_v<T>> {};

template <class T>
constexpr bool is_arithmetic_bound_v = is_arithmetic_bound<T>::value;
