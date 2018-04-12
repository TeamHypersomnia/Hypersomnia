#pragma once
#include <utility>

template <class StreamType, class T, class = void>
struct can_stream_left : std::false_type {

};

template <class StreamType, class T>
struct can_stream_left<StreamType, T, decltype(std::declval<StreamType&>() << std::declval<T>(), void())> : std::true_type {

};

template <class StreamType, class T, class = void>
struct can_stream_right : std::false_type {

};

template <class StreamType, class T>
struct can_stream_right<StreamType, T, decltype(std::declval<StreamType&>() >> std::declval<T&>(), void())> : std::true_type {

};

template <class StreamType, class T>
constexpr bool can_stream_left_v = can_stream_left<StreamType, T>::value;

template <class StreamType, class T>
constexpr bool can_stream_right_v = can_stream_right<StreamType, T>::value;