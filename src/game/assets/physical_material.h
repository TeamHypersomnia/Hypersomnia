#pragma once
#include <string>
#include <unordered_map>
#include "game/assets/ids/asset_ids.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

struct physical_material {
	using collision_sound_matrix_type = std::unordered_map<
		assets::physical_material_id, 
		assets::sound_id
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