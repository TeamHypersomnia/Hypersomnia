#pragma once
#include "game/resources/particle_effect.h"

namespace assets {
	enum class particle_effect_id {
		INVALID,

		PIXEL_BURST,
		PIXEL_PROJECTILE_TRACE,
		PIXEL_BARREL_LEAVE_EXPLOSION,
		ROUND_ROTATING_BLOOD_STREAM,
		WANDERING_PIXELS_DIRECTED,
		WANDERING_PIXELS_SPREAD,
		CONCENTRATED_WANDERING_PIXELS,
		PIXEL_METAL_SPARKLES,
		COUNT
	};
}

resources::particle_effect& operator*(const assets::particle_effect_id& id);