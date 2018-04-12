#pragma once
#include <type_traits>
#include "augs/templates/always_false.h"

namespace augs {
	struct introspection_access;
}

template <class T, class = void>
struct has_introspect : std::false_type {};

template <class T>
struct has_introspect<
	T,
	decltype(
		augs::introspection_access::introspect_body(
			static_cast<T*>(nullptr),
			true_returner(),
			std::declval<T&>()
		),
		void()
	)
> : std::true_type {
};

template <class T>
constexpr bool has_introspect_v = has_introspect<T>::value;

template <class T>
struct is_introspective_leaf : 
	std::bool_constant<
		std::is_enum_v<T>
		|| std::is_arithmetic_v<T>
	> 
{
};

template <class T>
constexpr bool is_introspective_leaf_v = is_introspective_leaf<T>::value;