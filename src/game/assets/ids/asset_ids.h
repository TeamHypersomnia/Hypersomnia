#pragma once
#include "augs/misc/pool/pooled_object_id.h"

using asset_pool_id_size_type = unsigned short;

template <class key>
using make_asset_pool_id = augs::pooled_object_id<asset_pool_id_size_type, key>;

namespace assets {
	/* 
		Create keys so that asset ids designate separate types,
		thus forbidding them from being used interchangeably 
	*/

	class image_id_key { image_id_key() = delete; };
	class animation_id_key { animation_id_key() = delete; };

	using image_id = make_asset_pool_id<image_id_key>;
	using animation_id = make_asset_pool_id<animation_id_key>;

	enum class sound_buffer_id;
	enum class particle_effect_id;
}
