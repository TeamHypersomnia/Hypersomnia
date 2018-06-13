#pragma once
#include "game/assets/ids/asset_ids.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

#include "game/detail/footstep_effect.h"

struct common_assets {
	// GEN INTROSPECTOR struct common_assets
	sound_effect_input ped_shield_impact_sound;
	sound_effect_input ped_shield_destruction_sound;
	sound_effect_input cast_unsuccessful_sound;

	sound_effect_input item_throw_sound;
	sound_effect_input item_holster_sound;
	sound_effect_input item_pickup_to_deposit_sound;

	particle_effect_input item_pickup_particles;
	particle_effect_input exhausted_smoke_particles;

	assets::particle_effect_id exploding_ring_smoke;
	assets::particle_effect_id exploding_ring_sparkles;
	assets::particle_effect_id thunder_remnants;

	footstep_effect_input standard_footstep;
	particle_effect_input haste_footstep_particles;
	// END GEN INTROSPECTOR
};