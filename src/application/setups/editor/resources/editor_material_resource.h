#pragma once
#include <cstdint>
#include "augs/math/vec2.h"
#include "game/assets/ids/asset_ids.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_particle_effect.h"
#include "application/setups/editor/resources/editor_sound_effect.h"
#include "test_scenes/test_scene_physical_materials.h"

struct editor_material_resource;

struct editor_collision_sound_def {
	using bound = augs::bound<real32>;

	/* 
		Note that "collider" field will just be left empty in default_collision 
		and will thus not get serialized.
	*/

	// GEN INTROSPECTOR struct editor_collision_sound_def
	editor_typed_resource_id<editor_material_resource> collider;

	editor_typed_resource_id<editor_sound_resource> sound;
	editor_particle_effect particles;
	real32 min_pitch = 0.9f;
	real32 max_pitch = 1.5f;

	real32 collision_sound_sensitivity = 1.0f;

	real32 min_interval_ms = 0.f;
	real32 unmute_after_ms = 250.f;
	uint32_t mute_after_playing_times = 4;

	bool silence_opposite_collision_sound = false;
	// END GEN INTROSPECTOR

	bool operator==(const editor_collision_sound_def&) const = default;
};

struct editor_material_resource_editable {
	// GEN INTROSPECTOR struct editor_material_resource_editable
	std::vector<editor_collision_sound_def> specific_collisions;

	editor_collision_sound_def default_collision;

	editor_sound_effect damage_sound;
	bool silence_damager_impact_sound = false;
	bool silence_damager_destruction_sound = false;

	editor_particle_effect damage_particles;

	real32 unit_damage_for_effects = 30.f;
	real32 max_ricochet_angle = 20.0f;
	bool point_blank_ricochets = false;
	real32 penetrability = 1.0f;

	editor_sound_effect destruction_sound;
	editor_particle_effect destruction_particles;
	// END GEN INTROSPECTOR

	bool operator==(const editor_material_resource_editable&) const = default;
};

struct editor_material_resource {
	editor_material_resource_editable editable;

	std::optional<test_scene_physical_material_id> official_tag;
	mutable assets::physical_material_id scene_asset_id;

	mutable uint32_t reference_count = 0u;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Material";
	}
};
