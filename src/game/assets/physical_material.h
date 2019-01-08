#pragma once
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
	bound pitch = bound(0.9f, 1.5f);
	real32 gain_mult = 1.f / 225.f;
	real32 pitch_mult = 1.f / 185.f;

	real32 cooldown_duration = 250.f;
	int occurences_before_cooldown = 4;
	// END GEN INTROSPECTOR
};

struct physical_material {
	using collision_sound_matrix_type = std::unordered_map<
		assets::physical_material_id, 
		collision_sound_def
	>;

	// GEN INTROSPECTOR struct physical_material
	collision_sound_matrix_type collision_sound_matrix;

	sound_effect_input standard_damage_sound;
	particle_effect_input standard_damage_particles;
	real32 unit_effect_damage = 30.f;
	std::string name;
	// END GEN INTROSPECTOR

	const auto& get_name() const {
		return name;
	}
};