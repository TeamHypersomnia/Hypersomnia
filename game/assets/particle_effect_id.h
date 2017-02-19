#pragma once
#include "game/resources/particle_effect.h"

namespace assets {
	enum class particle_effect_id : unsigned short {
		INVALID,

		PIXEL_BURST,
		PIXEL_PROJECTILE_TRACE,
		PIXEL_MUZZLE_LEAVE_EXPLOSION,
		ROUND_ROTATING_BLOOD_STREAM,
		WANDERING_PIXELS_DIRECTED,
		WANDERING_PIXELS_SPREAD,
		CONCENTRATED_WANDERING_PIXELS,
		PIXEL_METAL_SPARKLES,
		WANDERING_SMOKE,
		MUZZLE_SMOKE,
		ENGINE_PARTICLES,
		CAST_SPARKLES,
		CAST_CHARGING,
		EXHAUSTED_SMOKE,
		COUNT
	};
}

resources::particle_effect& operator*(const assets::particle_effect_id& id);