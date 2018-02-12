#pragma once
#include <type_traits>

template <class T>
struct always_false : std::false_type {};

template<class T>
constexpr auto always_false_v = always_false<T>::value;

template <class T>
struct always_true : std::true_type {};

template<class T>
constexpr auto always_true_v = always_true<T>::value;