#pragma once
#include <type_traits>
#include "augs/templates/predicate_templates.h"

template <class... T>
constexpr bool are_types_memcpy_safe_v = std::conjunction_v<std::is_trivially_copyable<T>...>;

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

template <class A, class = void>
struct allows_nontriviality : std::false_type {

};

template <class A>
struct allows_nontriviality<A, decltype(A::allow_nontriviality, void())> 
	: std::bool_constant<A::allow_nontriviality> 
{

};

template <class A>
constexpr bool allows_nontriviality_v = allows_nontriviality<A>::value;