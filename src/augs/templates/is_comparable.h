#pragma once
#include <type_traits>

template <class A, class B, class = void>
struct is_comparable : std::false_type {

};

template <class A, class B>
struct is_comparable<A, B, decltype(
	std::declval<A>() == std::declval<B>(),
	void()
	) > : std::bool_constant<!(std::is_array_v<A> && std::is_array_v<B>)> {

};

template <class A, class B>
static constexpr bool is_comparable_v = is_comparable<A, B>::value;
