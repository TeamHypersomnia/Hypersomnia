#pragma once
#include <optional>
#include "augs/templates/remove_cref.h"

template <class T>
constexpr bool is_nullopt_v = std::is_same_v<remove_cref<T>, std::nullopt_t>;

