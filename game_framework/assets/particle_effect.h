#pragma once
#include "../resources/particle_effect.h"

namespace assets {
	enum particle_effect_id {
		PIXEL_ROUND_BURST,
		PIXEL_ROUND_TRACE,
		PIXEL_BARREL_EXPLOSION,
		PIXEL_METAL_SPARKLES
	};
}

resources::particle_effect& operator*(const assets::particle_effect_id& id);