#pragma once
#include "augs/graphics/rgba.h"

#include "game/transcendental/entity_id.h"
#include "game/assets/ids/particle_effect_id.h"

struct particle_effect_modifier {
	// GEN INTROSPECTOR struct particle_effect_modifier
	rgba colorize;
	real32 scale_amounts = 1.f;
	real32 scale_lifetimes = 1.f;
	entity_id homing_target;
	// END GEN INTROSPECTOR
};	

struct particle_effect_input {
	// GEN INTROSPECTOR struct particle_effect_input
	assets::particle_effect_id id = assets::particle_effect_id::INVALID;
	particle_effect_modifier modifier;
	// END GEN INTROSPECTOR
};