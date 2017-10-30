#pragma once
#if STATICALLY_ALLOCATE_ASSETS
#include "augs/misc/enum/enum_associative_array.h"

template <class enum_key, class mapped>
using asset_map = augs::enum_associative_array<enum_key, mapped>;
#else
#include <unordered_map>
template <class enum_key, class mapped>
using asset_map = std::unordered_map<enum_key, mapped>;
#endif