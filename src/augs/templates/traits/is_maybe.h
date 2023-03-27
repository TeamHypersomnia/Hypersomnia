#pragma once
#include "augs/templates/maybe.h"

template <class T>
struct is_maybe : std::false_type {};

template <class T, bool S>
struct is_maybe<augs::maybe<T, S>> : std::true_type {};

template <class T>
static constexpr bool is_maybe_v = is_maybe<T>::value;