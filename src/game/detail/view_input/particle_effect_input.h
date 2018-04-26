#pragma once
#include "augs/graphics/rgba.h"

#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"

#include "game/assets/ids/asset_ids.h"
#include "game/detail/transform_copying.h"

struct particle_effect_modifier {
	// GEN INTROSPECTOR struct particle_effect_modifier
	rgba colorize;
	real32 scale_amounts = 1.f;
	real32 scale_lifetimes = 1.f;
	// END GEN INTROSPECTOR
};	

struct particle_effect_start_input {
	absolute_or_local positioning;
	entity_id homing_target;

	static particle_effect_start_input fire_and_forget(const components::transform where) {
		particle_effect_start_input	in;
		in.positioning.offset = where;
		return in; 
	}

	static particle_effect_start_input orbit_local(const entity_id id, const components::transform offset) {
		particle_effect_start_input	in;
		in.positioning = { id, offset };
		return in; 
	}

	static particle_effect_start_input orbit_absolute(const const_entity_handle h, components::transform offset);

	static particle_effect_start_input at_entity(const entity_id id) {
		return orbit_local(id, {});
	}

	auto& set_homing(const entity_id id) {
		homing_target = id;
		return *this;	
	}
};

struct particle_effect_input {
	// GEN INTROSPECTOR struct particle_effect_input
	assets::particle_effect_id id = assets::particle_effect_id::INVALID;
	particle_effect_modifier modifier;
	// END GEN INTROSPECTOR

	void start(logic_step, particle_effect_start_input) const;
};
