#pragma once

template <class T, class = void>
struct has_reference_count : std::false_type {};

template <class T>
struct has_reference_count<T, decltype(std::declval<T&>().reference_count, void())> : std::true_type {};

template <class T>
constexpr bool has_reference_count_v = has_reference_count<T>::value;
