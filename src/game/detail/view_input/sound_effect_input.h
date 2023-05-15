#pragma once
#include "augs/misc/constant_size_vector.h"
#include "view/view_container_sizes.h"
#include "augs/pad_bytes.h"
#include "game/assets/ids/asset_ids.h"
#include "augs/templates/hash_templates.h"
#include "game/components/transform_component.h"
#include "game/detail/transform_copying.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"
#include "game/enums/faction_type.h"
#include "game/detail/view_input/predictability_info.h"
#include "game/detail/view_input/sound_effect_modifier.h"

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

struct sound_effect_start_input {
	absolute_or_local positioning;
	entity_id direct_listener;
	faction_type listener_faction = faction_type::SPECTATOR;
	std::size_t variation_number = static_cast<std::size_t>(-1);
	collision_sound_source source_collision;

	real32 collision_min_interval_ms = 50.f;
	real32 collision_unmute_after_ms = 250.f;
	int collision_mute_after_playing_times = 4;

	bool clear_when_target_entity_deleted = false;
	bool clear_when_target_alive = false;
	bool clear_when_target_conscious = false;
	bool silent_trace_like = false;

	bool is_missile_impact() const {
		return collision_mute_after_playing_times < 0;
	}

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

	auto& face_velocity() {
		positioning.face_velocity = true;
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

	auto& mark_as_missile_impact(const entity_id sub, const entity_id col) {
		source_collision.subject = sub;
		source_collision.collider = col;
		collision_mute_after_playing_times = -1;

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

	void start(const_logic_step, sound_effect_start_input, predictability_info) const;
};

struct packaged_sound_effect {
	sound_effect_input input;
	sound_effect_start_input start;

	void post(const_logic_step step, predictability_info info) const;
};

using sound_effect_input_vector = augs::constant_size_vector<sound_effect_input, MAX_FOLLOWUP_SOUND_INPUTS>;

struct packaged_multi_sound_effect {
	sound_effect_input_vector inputs;
	sound_effect_start_input start;
};