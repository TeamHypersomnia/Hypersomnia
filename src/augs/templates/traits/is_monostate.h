#pragma once
#include <variant>
#include "augs/templates/remove_cref.h"

template <class T>
constexpr bool is_monostate_v = std::is_same_v<remove_cref<T>, std::monostate>;

