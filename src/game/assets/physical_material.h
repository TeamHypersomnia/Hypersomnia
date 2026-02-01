#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include "game/assets/ids/asset_ids.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "augs/misc/bound.h"

struct collision_sound_def {
	using bound = augs::bound<real32>;

	// GEN INTROSPECTOR struct collision_sound_def
	sound_effect_input effect;
	particle_effect_input particles;
	bound pitch_bound = bound(0.9f, 1.5f);

	real32 collision_sound_sensitivity = 1.0f;

	real32 min_interval_ms = 0.f;
	real32 unmute_after_ms = 250.f;
	uint32_t mute_after_playing_times = 4;

	bool silence_opposite_collision_sound = false;
	// END GEN INTROSPECTOR
};

struct physical_material {
	using collision_sound_matrix_type = std::unordered_map<
		assets::physical_material_id, 
		collision_sound_def
	>;

	// GEN INTROSPECTOR struct physical_material
	collision_sound_matrix_type collision_sound_matrix;

	collision_sound_def default_collision;

	sound_effect_input standard_damage_sound;
	bool silence_damager_impact_sound = false;
	bool silence_damager_destruction_sound = false;

	particle_effect_input standard_damage_particles;
	real32 unit_damage_for_effects = 30.f;

	sound_effect_input standard_destruction_sound;
	particle_effect_input standard_destruction_particles;
	// END GEN INTROSPECTOR

	/* Compatibility dummy */
	std::string get_name() const {
		return "Material";
	}
};