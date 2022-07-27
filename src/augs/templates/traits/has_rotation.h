#pragma once

template <class T, class = void>
struct has_rotation : std::false_type {};

template <class T>
struct has_rotation<T, decltype(std::declval<T&>().rotation, void())> : std::true_type {};

template <class T>
constexpr bool has_rotation_v = has_rotation<T>::value;
