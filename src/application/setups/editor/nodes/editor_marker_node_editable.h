#pragma once
#include "application/setups/editor/resources/editor_particle_effect.h"
#include "application/setups/editor/resources/editor_sound_effect.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "game/components/marker_component.h"

#include "game/detail/sentience_shake.h"

struct editor_point_marker_node;

struct editor_filter_flags {
	// GEN INTROSPECTOR struct editor_filter_flags
	bool characters = true;
	bool bullets = false;
	bool flying_explosives = false;
	bool flying_melees = false;
	bool lying_items = true;
	bool shells = true;
	bool obstacles = false;
	// END GEN INTROSPECTOR

	bool operator==(const editor_filter_flags& b) const = default;

	uint16_t get_mask_bits() const;
};

struct editor_portal_info {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_portal_info
	bool quiet_entry = false;
	bool quiet_exit = false;

	bool trampoline_like = false;
	bool preserve_entry_offset = false;

	float enter_time_ms = 1000.0f;
	float travel_time_ms = 1000.0f;
	float exit_cooldown_ms = 200.0f;

	editor_filter_flags reacts_to;

	sentience_shake enter_shake = { 1000.0f, 1.0f };
	sentience_shake exit_shake = { 1000.0f, 1.0f };

	editor_sound_effect begin_entering_sound;
	editor_sound_effect enter_sound;
	editor_sound_effect exit_sound;

	editor_particle_effect begin_entering_particles;
	editor_particle_effect enter_particles;
	editor_particle_effect exit_particles;

	editor_typed_node_id<editor_point_marker_node> portal_exit;
	portal_exit_impulses exit_impulses;
	// END GEN INTROSPECTOR
};

struct editor_point_marker_node_editable {
	// GEN INTROSPECTOR struct editor_point_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;

	vec2 pos;
	real32 rotation = 0.0f;
	// END GEN INTROSPECTOR
};

struct editor_area_marker_node_editable {
	// GEN INTROSPECTOR struct editor_area_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;

	vec2 pos;
	real32 rotation = 0.0f;
	vec2i size = { 256, 256 };

	editor_portal_info as_portal;
	// END GEN INTROSPECTOR
};
