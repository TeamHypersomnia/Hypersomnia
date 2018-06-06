#pragma once

#include "augs/pad_bytes.h"
#include "game/assets/ids/asset_ids.h"
#include "augs/templates/hash_templates.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/transform_component.h"
#include "game/detail/transform_copying.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

struct sound_effect_modifier {
	// GEN INTROSPECTOR struct sound_effect_modifier
	float gain = 1.f;
	float pitch = 1.f;
	float max_distance = 1920.f * 3.f;
	float reference_distance = 0.f;
	short repetitions = 1;
	bool fade_on_exit = true;
	pad_bytes<1> pad;
	// END GEN INTROSPECTOR
};

struct sound_effect_start_input {
	absolute_or_local positioning;
	entity_id direct_listener;
	std::size_t variation_number = static_cast<std::size_t>(-1);

	static sound_effect_start_input fire_and_forget(const components::transform where) {
		sound_effect_start_input in;
		in.positioning.offset = where;
		return in; 
	}

	static sound_effect_start_input orbit_local(const entity_id id, const components::transform offset) {
		sound_effect_start_input in;
		in.positioning = { id, offset };
		return in; 
	}

	static sound_effect_start_input orbit_absolute(const const_entity_handle h, components::transform offset);

	auto& set_listener(const entity_id id) {
		direct_listener = id;
		return *this;	
	}

	static sound_effect_start_input at_entity(const entity_id id) {
		return orbit_local(id, {});
	}

	static sound_effect_start_input at_listener(const entity_id id) {
		return at_entity(id).set_listener(id);
	}
};

struct sound_effect_input {
	// GEN INTROSPECTOR struct sound_effect_input
	assets::sound_id id;
	sound_effect_modifier modifier;
	// END GEN INTROSPECTOR

	float calc_max_audible_distance() const {
		return modifier.max_distance + modifier.reference_distance;
	}

	void start(logic_step, sound_effect_start_input) const;
};

struct packaged_sound_effect {
	sound_effect_input input;
	sound_effect_start_input start;

	void post(logic_step step) const;
};