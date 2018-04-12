#pragma once

struct true_returner {
	template <class... Types>
	bool operator()(Types...) const {
		return true;
	}
};

template <class T>
struct always_false {
	static constexpr bool value = false;
};

template<class T>
constexpr auto always_false_v = always_false<T>::value;

template <class T>
struct always_true {
	static constexpr bool value = true;
};

template<class T>
constexpr auto always_true_v = always_true<T>::value;