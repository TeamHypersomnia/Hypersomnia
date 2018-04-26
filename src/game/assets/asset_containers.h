#pragma once
#include "game/assets/ids/asset_ids.h"
#include "game/container_sizes.h"

#include "augs/misc/pool/pool.h"

#if STATICALLY_ALLOCATE_ASSETS
#include "augs/misc/enum/enum_map.h"
#include "augs/misc/constant_size_vector.h"

template <class pooled_type, std::size_t MAX_COUNT, class... keys>
using make_asset_pool = augs::pool<
	pooled_type,
	of_size<MAX_IMAGE_COUNT>::make_constant_vector,
	asset_pool_id_size_type,
	keys...
>;

template <class enum_key, class mapped>
using asset_map = augs::enum_map<enum_key, mapped>;

#else

template <class enum_key, class mapped>
using asset_map = std::unordered_map<enum_key, mapped>;

template <class pooled_type, std::size_t MAX_COUNT, class... keys>
using make_asset_pool = augs::pool<
	pooled_type,
	make_vector,
	asset_pool_id_size_type,
	keys...
>;

#endif

template <class pooled_type>
using image_id_pool = make_asset_pool<pooled_type, MAX_IMAGE_COUNT, assets::image_id_key>;

template <class pooled_type>
using sound_id_pool = make_asset_pool<pooled_type, MAX_SOUND_BUFFER_COUNT, assets::sound_id_key>;

struct animation;
using animations_pool = make_asset_pool<animation, MAX_ANIMATION_COUNT, assets::animation_id_key>;