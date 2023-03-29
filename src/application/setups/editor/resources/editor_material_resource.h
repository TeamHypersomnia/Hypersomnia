#pragma once
#include "augs/math/vec2.h"
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_particle_effect.h"
#include "application/setups/editor/resources/editor_sound_effect.h"
#include "test_scenes/test_scene_physical_materials.h"

struct editor_material_resource;

struct editor_collision_sound_def {
	using bound = augs::bound<real32>;

	// GEN INTROSPECTOR struct editor_collision_sound_def
	editor_sound_effect effect;
	bound pitch = bound(0.9f, 1.5f);
	real32 gain_mult = 1.f / 225.f;
	real32 pitch_mult = 1.f / 185.f;

	real32 cooldown_duration = 250.f;
	int occurences_before_cooldown = 4;
	// END GEN INTROSPECTOR
};

struct editor_material_resource_editable {
	using collision_sound_matrix_type = std::vector<augs::simple_pair<
		editor_typed_resource_id<editor_material_resource>, 
		editor_collision_sound_def
	>>;

	// GEN INTROSPECTOR struct editor_material_resource_editable
	collision_sound_matrix_type collision_sound_matrix;

	editor_sound_effect standard_damage_sound;
	editor_particle_effect standard_damage_particles;

	real32 unit_effect_damage = 30.f;
	real32 max_ricochet_angle = 20.0f;
	// END GEN INTROSPECTOR
};

struct editor_material_resource {
	editor_material_resource_editable editable;

	std::optional<test_scene_physical_material_id> official_tag;
	mutable assets::physical_material_id scene_asset_id;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Material";
	}
};
