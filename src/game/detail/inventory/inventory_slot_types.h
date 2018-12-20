#pragma once
#include <type_traits>

using inventory_space_type = unsigned;

constexpr inventory_space_type max_inventory_space_v = static_cast<unsigned>(-1);
static_assert(std::is_same_v<inventory_space_type, unsigned>, "If you change the type, do something about the maximum value first.");
