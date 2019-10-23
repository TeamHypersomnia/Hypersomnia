#pragma once
#include "augs/templates/always_false.h"

struct empty_callback {
	template <class... Types>
	void operator()(Types&&...) const {}
};

struct true_returner {
	template <class... Types>
	bool operator()(Types&&...) const {
		return true;
	}
};

template <class T>
struct always_true {
	static constexpr bool value = true;
};

template<class T>
constexpr bool always_true_v = always_true<T>::value;