#pragma once
#include <type_traits>
#include <xtr1common>

namespace augs {
	template <class T, class F>
	decltype(auto) introspect(
		T& t,
		F f
	) {
		return introspect<std::is_const<T>::value>(t, f);
	}
}

struct true_returner {
	template <class T>
	bool operator()(T) const {
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
	std::enable_if_t<
		std::is_same<
			std::true_type,

			decltype(
				augs::introspect<C>(
					std::declval<
						std::conditional_t<C, const T, T>
					>(),
					true_returner()
				), 
				std::true_type()
			)
		>::value
	>
> {
	static constexpr bool value = true;
};

template <class T>
struct has_introspects {
	static constexpr bool value = has_introspect<T, false>::value && has_introspect<T, true>::value;
};