#pragma once
#include <type_traits>

template <class A, class = void>
struct is_synchronized : std::false_type {};

template <class A>
struct is_synchronized<A, decltype(A::is_synchronized, void())> 
	: std::bool_constant<A::is_synchronized> 
{};

template <class A, class = void>
struct is_always_present : std::false_type {};

template <class A>
struct is_always_present<A, decltype(A::is_always_present, void())> 
	: std::bool_constant<A::is_always_present>
{};

template <class A>
constexpr bool is_synchronized_v = is_synchronized<A>::value;

template <class A>
constexpr bool is_always_present_v = is_always_present<A>::value;

template <class T, class = void>
struct has_implied_component : std::false_type {};

template <class T>
struct has_implied_component<T, decltype(typename T::implied_component(), void())> : std::true_type {};

template <class A>
constexpr bool has_implied_component_v = has_implied_component<A>::value;