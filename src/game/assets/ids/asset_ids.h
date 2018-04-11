#pragma once
#include "augs/misc/pool/pooled_object_id.h"

using asset_pool_id_size_type = unsigned short;
using asset_pool_id = augs::pooled_object_id<asset_pool_id_size_type>;

namespace assets {
	using image_id = asset_pool_id;
	using animation_id = asset_pool_id;

	enum class sound_buffer_id;
	enum class particle_effect_id;
}
