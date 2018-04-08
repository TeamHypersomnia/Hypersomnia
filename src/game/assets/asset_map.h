#pragma once
#if STATICALLY_ALLOCATE_ASSETS

#include "augs/misc/enum/enum_map.h"
#include "game/assets/ids/asset_id_declarations.h"
#include "game/container_sizes.h"

template <class enum_key, class mapped>
using asset_map = augs::enum_map<enum_key, mapped>;

#include <unordered_map>
template <class mapped>
using image_id_map = std::unordered_map<assets::image_id, mapped>;

#else

#include <unordered_map>

template <class enum_key, class mapped>
using asset_map = std::unordered_map<enum_key, mapped>;

template <class mapped>
using image_id_map = std::unordered_map<assets::image_id, mapped>;

#endif