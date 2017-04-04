#pragma once
#include "game/resources/particle_effect.h"

namespace assets {
	enum class particle_effect_id : unsigned {
		HEALTH_DAMAGE_SPARKLES,
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
		THUNDER_REMNANTS,
		COUNT
	};
}

resources::particle_effect& operator*(const assets::particle_effect_id& id);

struct _particle_effect_response {
	// GEN INTROSPECTOR struct _particle_effect_response
	assets::particle_effect_id id = assets::particle_effect_id::COUNT;
	particle_effect_modifier modifier;
	// END GEN INTROSPECTOR
};