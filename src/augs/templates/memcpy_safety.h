#pragma once
#include <type_traits>
#include "augs/templates/predicate_templates.h"

template<class... _Ty>
constexpr bool are_types_memcpy_safe_v 
	= typename make_variadic_predicate<
		std::conjunction, 
		std::is_trivially_copyable 
	>::type<_Ty...>::value
;

template<class _Ty>
constexpr bool is_memcpy_safe_v = are_types_memcpy_safe_v<_Ty>;

template <class A>
bool trivial_compare(const A& a, const A& b) {
	static_assert(is_memcpy_safe_v<A>, "Type can't be trivially compared!");
	return !std::memcmp(&a, &b, sizeof(A));
}

template <class A>
bool trivial_compare(const A* const a, const A* const b) {
	static_assert(is_memcpy_safe_v<A>, "Type can't be trivially compared!");
	return !std::memcmp(a, b, sizeof(A));
}