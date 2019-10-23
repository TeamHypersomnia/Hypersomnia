#pragma once

template <class T>
struct always_false {
	static constexpr bool value = false;
};

template<class T>
constexpr bool always_false_v = always_false<T>::value;

