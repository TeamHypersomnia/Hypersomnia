#pragma once
#include <type_traits>
#include <xtr1common>
#include "augs/templates/maybe_const.h"

namespace augs {
	template <class T, class F>
	decltype(auto) introspect(
		T& t,
		F f
	) {
		return introspect<std::is_const_v<T>>(t, f);
	}
}

struct true_returner {
	template <class... Types>
	bool operator()(Types...) const {
		return true;
	}
};

template <class T, bool C, class = void>
struct has_introspect {
	static constexpr bool value = false;
};

template <class T, bool C>
struct has_introspect<
	T, 
	C,
	decltype(
		augs::introspect<C>(
			std::declval<
				std::conditional_t<C, const T, T>
			>(),
			true_returner()
		), 
		void()
	)
> {
	static constexpr bool value = true;
};

template <class T, bool C>
constexpr bool has_introspect_v = has_introspect<T, C>::value;

template <class T>
struct has_introspects {
	static constexpr bool value = has_introspect_v<T, false> && has_introspect_v<T, true>;
}; 

template <class T>
constexpr bool has_introspects_v = has_introspects<T>::value;
