#pragma once
#if STATICALLY_ALLOCATE_ASSETS

#include "augs/misc/enum/enum_map.h"
#include "game/assets/ids/asset_ids.h"
#include "game/container_sizes.h"
#include "augs/misc/pool/pool.h"
#include "augs/misc/declare_containers.h"

template <class enum_key, class mapped>
using asset_map = augs::enum_map<enum_key, mapped>;

template <class pooled_type, std::size_t MAX_COUNT>
using make_asset_pool = augs::pool<
	pooled_type,
	of_size<MAX_IMAGE_COUNT>::make_constant_vector,
	asset_pool_id_size_type
>

#else

template <class enum_key, class mapped>
using asset_map = std::unordered_map<enum_key, mapped>;

template <class pooled_type, std::size_t MAX_COUNT>
using make_asset_pool = augs::pool<
	pooled_type,
	make_vector,
	asset_pool_id_size_type
>

#endif

template <class pooled_type>
using image_id_pool = make_asset_pool<pooled_type, MAX_IMAGE_COUNT>;