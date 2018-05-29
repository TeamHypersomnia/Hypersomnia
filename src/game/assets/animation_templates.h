#pragma once
#include <type_traits>

template <class T, class = void>
struct has_frames : std::false_type {};

template <class T>
struct has_frames<T, decltype(std::declval<T&>().frames, void())> : std::true_type {};

template <class T>
constexpr bool has_frames_v = has_frames<T>::value; 

template <class T>
using frame_type_t = std::remove_reference_t<decltype(std::declval<T&>().frames[0])>;
