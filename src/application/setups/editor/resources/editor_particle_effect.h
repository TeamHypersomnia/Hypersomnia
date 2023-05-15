#pragma once
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "game/detail/view_input/particle_effect_modifier.h"

struct editor_particle_effect : particle_effect_modifier {
	using base = particle_effect_modifier;
	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_particle_effect
	editor_typed_resource_id<editor_particles_resource> id;
	// END GEN INTROSPECTOR

	bool operator==(const editor_particle_effect&) const = default;
};

