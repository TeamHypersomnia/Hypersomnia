#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/allocate_new_entity_access.h"

struct blood_splatter_params {
	real32 damage_per_splatter = 40.f;
	real32 min_size = 0.5f;
	real32 angle_spread = 40.f;
	real32 min_distance = 0.f;
	real32 max_distance_base = 30.f;
	real32 distance_damage_scale = 200.f;
	real32 max_distance_at_full_damage = 150.f;
};

/*
	Spawns a single blood splatter at an exact position with specified size.
	Use this for precise control over splatter placement.
*/
void spawn_blood_splatter(
	allocate_new_entity_access access,
	const logic_step step,
	const entity_id subject,
	const vec2 splatter_position,
	const vec2 burst_origin,
	const real32 size_mult
);

void spawn_blood_splatters(
	allocate_new_entity_access access,
	const logic_step step,
	const entity_id subject,
	const vec2 position,
	const vec2 impact_direction,
	const real32 damage_amount,
	const blood_splatter_params& params = blood_splatter_params()
);

void spawn_blood_splatters_omnidirectional(
	allocate_new_entity_access access,
	const logic_step step,
	const entity_id subject,
	const vec2 position,
	const real32 damage_amount
);
