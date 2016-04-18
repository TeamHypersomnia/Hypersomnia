#pragma once
#include "../resources/particle_effect.h"

namespace assets {
	enum particle_effect_id {
		PIXEL_BURST,
		PIXEL_PROJECTILE_TRACE,
		PIXEL_BARREL_LEAVE_EXPLOSION,
		PIXEL_METAL_SPARKLES
	};
}

resources::particle_effect& operator*(const assets::particle_effect_id& id);