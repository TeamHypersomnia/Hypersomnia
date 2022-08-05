#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_particles_resource.h"

struct editor_particles_node_editable : particle_effect_modifier {
	using base = particle_effect_modifier;
	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_particles_node_editable
	vec2 pos;
	// END GEN INTROSPECTOR
};

struct editor_particles_node {
	editor_typed_resource_id<editor_particles_resource> resource_id;
	editor_particles_node_editable editable;
	bool visible = true;

	mutable entity_id scene_entity_id;

	auto get_transform() const {
		return transformr(editable.pos, 0.0f);
	}

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}
};
