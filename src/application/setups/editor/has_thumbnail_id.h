#pragma once

template <class T, class = void>
struct has_thumbnail_id : std::false_type {};

template <class T>
struct has_thumbnail_id<T, decltype(std::declval<T&>().thumbnail_id, void())> : std::true_type {};

template <class T>
constexpr bool has_thumbnail_id_v = has_thumbnail_id<T>::value;
