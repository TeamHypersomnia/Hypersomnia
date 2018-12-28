#pragma once
#include "augs/graphics/rgba.h"

#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"

#include "game/assets/ids/asset_ids.h"
#include "game/detail/transform_copying.h"
#include "game/detail/view_input/particle_effect_modifier.h"
#include "game/detail/view_input/predictability_info.h"

struct particle_effect_start_input {
	absolute_or_local positioning;
	entity_id homing_target;
	predictability_info predictability;
	bool stream_infinitely = false;

	static particle_effect_start_input fire_and_forget(const transformr where) {
		particle_effect_start_input	in;
		in.positioning.offset = where;
		return in; 
	}

	static particle_effect_start_input orbit_local(const entity_id id, const transformr offset) {
		particle_effect_start_input	in;
		in.positioning = { id, offset };
		return in; 
	}

	static particle_effect_start_input orbit_absolute(const const_entity_handle h, transformr offset);

	static particle_effect_start_input at_entity(const entity_id id) {
		return orbit_local(id, {});
	}

	auto& predictable_only_by(const entity_id id) {
		predictability.set_only_by(id);
		return *this;
	}

	auto& never_predictable() {
		predictability.type = predictability_type::NEVER;
		return *this;
	}

	auto& set_homing(const entity_id id) {
		homing_target = id;
		return *this;	
	}
};

struct particle_effect_input {
	// GEN INTROSPECTOR struct particle_effect_input
	assets::particle_effect_id id;
	particle_effect_modifier modifier;
	// END GEN INTROSPECTOR

	void start(const_logic_step, particle_effect_start_input) const;
};

struct packaged_particle_effect {
	particle_effect_input input;
	particle_effect_start_input start;

	void post(const_logic_step step) const;
};
