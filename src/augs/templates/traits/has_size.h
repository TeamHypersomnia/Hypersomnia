#pragma once

template <class T, class = void>
struct has_size : std::false_type {};

template <class T>
struct has_size<T, decltype(std::declval<T&>().size, void())> : std::true_type {};

template <class T>
constexpr bool has_size_v = has_size<T>::value;
