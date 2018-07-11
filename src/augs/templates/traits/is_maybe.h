#pragma once
#include "augs/templates/maybe.h"

template <class T>
struct is_maybe : std::false_type {};

template <class T>
struct is_maybe<augs::maybe<T>> : std::true_type {};

template <class T>
static constexpr bool is_maybe_v = is_maybe<T>::value;