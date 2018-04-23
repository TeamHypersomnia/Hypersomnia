#pragma once
#include "augs/templates/value_with_flag.h"

template <class T>
struct is_value_with_flag : std::false_type {};

template <class T>
struct is_value_with_flag<augs::value_with_flag<T>> : std::true_type {};

template <class T>
static constexpr bool is_value_with_flag_v = is_value_with_flag<T>::value;