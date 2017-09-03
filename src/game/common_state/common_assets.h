#pragma once
#include "game/assets/particle_effect_id.h"
#include "game/assets/sound_buffer_id.h"
#include "game/assets/game_image_id.h"

struct common_assets {
	// GEN INTROSPECTOR struct common_assets
	sound_effect_input ped_shield_impact_sound;
	sound_effect_input ped_shield_destruction_sound;
	sound_effect_input cast_unsuccessful_sound;

	sound_effect_input item_throw_sound;

	particle_effect_input exhausted_smoke_particles;

	assets::particle_effect_id exploding_ring_smoke = assets::particle_effect_id::INVALID;
	assets::particle_effect_id exploding_ring_sparkles = assets::particle_effect_id::INVALID;
	assets::particle_effect_id thunder_remnants = assets::particle_effect_id::INVALID;
	// END GEN INTROSPECTOR
};