#pragma once

template <class T, class = void>
struct has_flip : std::false_type {};

template <class T>
struct has_flip<T, decltype(std::declval<T&>().flip_horizontally, void())> : std::true_type {};

template <class T>
constexpr bool has_flip_v = has_flip<T>::value;
