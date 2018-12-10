#pragma once
#include "augs/audio/distance_model.h"
#include "augs/misc/constant_size_vector.h"
#include "view/view_container_sizes.h"
#include "augs/pad_bytes.h"
#include "game/assets/ids/asset_ids.h"
#include "augs/templates/hash_templates.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/transform_component.h"
#include "game/detail/transform_copying.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"
#include "game/enums/faction_type.h"

struct sound_effect_modifier {
	// GEN INTROSPECTOR struct sound_effect_modifier
	real32 gain = 1.f;
	real32 pitch = 1.f;
	real32 max_distance = 1920.f * 3.f;
	real32 reference_distance = 0.f;
	real32 rolloff_factor = 1.f;
	real32 doppler_factor = 1.f;
	char repetitions = 1;
	bool fade_on_exit = true;
	bool sync_against_born_time = false;
	bool always_direct_listener = false;
	augs::distance_model distance_model = augs::distance_model::LINEAR_DISTANCE_CLAMPED;
	// END GEN INTROSPECTOR
};

struct collision_sound_source {
	entity_id subject;
	entity_id collider;

	bool operator==(const collision_sound_source& b) const {
		return 
			(subject == b.subject && collider == b.collider)
			|| (subject == b.collider && collider == b.subject)
		;
	}
};

namespace std {
	template <>
	struct hash<collision_sound_source> {
		std::size_t operator()(const collision_sound_source& v) const {
			return std::hash<entity_id>()(v.subject) + std::hash<entity_id>()(v.collider);
		}
	};
}

struct sound_effect_start_input {
	absolute_or_local positioning;
	entity_id direct_listener;
	bool clear_when_target_dead = false;
	faction_type listener_faction = faction_type::SPECTATOR;
	std::size_t variation_number = static_cast<std::size_t>(-1);
	collision_sound_source source_collision;
	real32 collision_sound_cooldown_duration = 250.f;
	int collision_sound_occurences_before_cooldown = 4;

	static sound_effect_start_input fire_and_forget(const transformr where) {
		sound_effect_start_input in;
		in.positioning.offset = where;
		return in; 
	}

	static sound_effect_start_input orbit_local(const entity_id id, const transformr offset) {
		sound_effect_start_input in;
		in.positioning = { id, offset };
		return in; 
	}

	static sound_effect_start_input orbit_absolute(const const_entity_handle h, transformr offset);

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

	auto& mark_source_collision(const entity_id sub, const entity_id col) {
		source_collision.subject = sub;
		source_collision.collider = col;

		return *this;
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

	void start(const_logic_step, sound_effect_start_input) const;
};

struct packaged_sound_effect {
	sound_effect_input input;
	sound_effect_start_input start;

	void post(const_logic_step step) const;
};

using sound_effect_input_vector = augs::constant_size_vector<sound_effect_input, MAX_FOLLOWUP_SOUND_INPUTS>;

struct packaged_multi_sound_effect {
	sound_effect_input_vector inputs;
	sound_effect_start_input start;

	void post(const_logic_step step) const;
};