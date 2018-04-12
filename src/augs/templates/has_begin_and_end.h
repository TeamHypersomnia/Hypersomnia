#pragma once
#include <utility>

template <class T, class = void>
struct has_begin_and_end : std::false_type {};

template <class T>
struct has_begin_and_end<T, decltype(std::declval<T&>().begin(), std::declval<T&>().end(), void())> : std::true_type {};

template <class T>
constexpr bool has_begin_and_end_v = has_begin_and_end<T>::value;

