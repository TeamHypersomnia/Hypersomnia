#pragma once
#include <utility>
#include "augs/misc/simple_pair.h"

template <typename T>
struct is_pair : std::false_type {};

template <typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : std::true_type {};

template <typename T1, typename T2>
struct is_pair<augs::simple_pair<T1, T2>> : std::true_type {};

template <typename T>
static constexpr bool is_pair_v = is_pair<T>::value;

