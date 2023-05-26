#pragma once

template <class T, class = void>
struct has_custom_scene_flavour_id : std::false_type {};

template <class T>
struct has_custom_scene_flavour_id<T, decltype(std::declval<T&>().custom_scene_flavour_id, void())> : std::true_type {};

template <class T>
constexpr bool has_custom_scene_flavour_id_v = has_custom_scene_flavour_id<T>::value;
