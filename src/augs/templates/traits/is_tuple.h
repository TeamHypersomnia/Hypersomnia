#pragma once
#include <tuple>

template <class T>
struct is_tuple : std::false_type {};

template <class... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <class T>
static constexpr bool is_tuple_v = is_tuple<T>::value;