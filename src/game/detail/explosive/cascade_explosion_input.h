#pragma once
#include "game/cosmos/entity_flavour_id.h"
#include "game/components/cascade_explosion_component.h"
#include "augs/templates/variated.h"

using cascade_explosion_flavour_id = constrained_entity_flavour_id<
	invariants::cascade_explosion
>;

struct cascade_explosion_input {
	// GEN INTROSPECTOR struct cascade_explosion_input
	cascade_explosion_flavour_id flavour_id;
	unsigned num_spawned = 8;
	augs::mult_variated<real32> initial_speed = { 1000.f, 0.2f };
	augs::variated<int> num_explosions = { 14, 5 };
	real32 spawn_spread = 360.f;
	real32 spawn_angle_variation = 0.2f;
	// END GEN INTROSPECTOR
};
