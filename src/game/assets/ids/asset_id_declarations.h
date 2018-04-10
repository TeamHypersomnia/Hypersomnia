#pragma once
#include "augs/misc/relinked_pool_id.h"

namespace assets {
	using image_id = relinked_pool_id<unsigned>;
	enum class sound_buffer_id;
	enum class particle_effect_id;
}
