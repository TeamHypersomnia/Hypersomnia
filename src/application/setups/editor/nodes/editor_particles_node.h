#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_particles_resource.h"

struct editor_particles_node_editable : particle_effect_modifier {
	using base = particle_effect_modifier;
	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_particles_node_editable
	vec2 pos;
	// END GEN INTROSPECTOR
};

struct editor_particles_node : editor_node_base<
	editor_particles_resource,
	editor_particles_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, 0.0f);
	}

	static const char* get_type_name() {
		return "Particles";
	}
};
