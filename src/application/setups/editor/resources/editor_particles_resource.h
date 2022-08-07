#pragma once
#include "augs/math/vec2.h"
#include "game/assets/ids/asset_ids.h"
#include "view/viewables/particle_effect.h"
#include "game/detail/view_input/particle_effect_modifier.h"
#include "game/cosmos/entity_flavour_id.h"

struct editor_particles_resource_editable : particle_effect_modifier {
	using base = particle_effect_modifier;
	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_particles_resource_editable
	emission_vector emissions;
	// END GEN INTROSPECTOR
};

struct editor_particles_node;

struct editor_particles_resource {
	using node_type = editor_particles_node;

	editor_particles_resource_editable editable;

	mutable std::variant<
		typed_entity_flavour_id<particles_decoration>
	> scene_flavour_id;

	mutable assets::particle_effect_id scene_asset_id;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}
};
