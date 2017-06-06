#pragma once
#include <type_traits>

template <class A, class = void>
struct is_component_synchronized : std::false_type {};

template <class A>
struct is_component_synchronized<A, decltype(A::is_synchronized, void())> 
	: std::bool_constant<A::is_synchronized> 
{};

template <class A, class = void>
struct is_component_fundamental : std::false_type {};

template <class A>
struct is_component_fundamental<A, decltype(A::is_fundamental, void())> 
	: std::bool_constant<A::is_fundamental>
{};

template <class A>
constexpr bool is_component_synchronized_v = is_component_synchronized<A>::value;

template <class A>
constexpr bool is_component_fundamental_v = is_component_fundamental<A>::value;