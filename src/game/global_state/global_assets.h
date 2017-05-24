#pragma once
#include "game/assets/particle_effect_id.h"
#include "game/assets/sound_buffer_id.h"
#include "game/assets/game_image_id.h"

struct global_assets {
	// GEN INTROSPECTOR struct global_assets
	sound_response ped_shield_impact_sound;
	sound_response ped_shield_destruction_sound;
	sound_response cast_unsuccessful_sound;

	particle_effect_response exhausted_smoke_particles;
	// END GEN INTROSPECTOR
};