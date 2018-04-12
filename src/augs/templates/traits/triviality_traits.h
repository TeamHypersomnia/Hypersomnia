#pragma once
#include <type_traits>
#include <cstring>

template <class A>
bool trivial_compare(const A& a, const A& b) {
	static_assert(std::is_trivially_copyable_v<A>, "Type can't be trivially compared!");
	return !std::memcmp(&a, &b, sizeof(A));
}

template <class A>
bool trivial_compare(const A* const a, const A* const b) {
	static_assert(std::is_trivially_copyable_v<A>, "Type can't be trivially compared!");
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