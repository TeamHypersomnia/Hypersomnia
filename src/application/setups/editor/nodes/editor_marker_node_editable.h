#pragma once
#include "application/setups/editor/resources/editor_particle_effect.h"
#include "application/setups/editor/resources/editor_sound_effect.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"

struct editor_point_marker_node;

struct editor_filter_flags {
	// GEN INTROSPECTOR struct editor_filter_flags
	bool characters = true;
	bool bullets = false;
	bool flying_explosives = false;
	bool flying_melees = false;
	bool lying_items = true;
	bool obstacles = false;
	// END GEN INTROSPECTOR

	bool operator==(const editor_filter_flags& b) const = default;
};

struct editor_portal_info {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_portal_info
	float enter_time_ms = 1000.0f;
	float travel_time_ms = 1000.0f;

	editor_sound_effect enter_sound;
	editor_sound_effect travel_sound;
	editor_sound_effect exit_sound;

	editor_particle_effect travel_particles;
	editor_particle_effect exit_particles;

	editor_typed_node_id<editor_point_marker_node> portal_exit;

	editor_filter_flags reacts_to;

	bool seamless_portal = false;
	// END GEN INTROSPECTOR
};

struct editor_portal_exit_info {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_portal_exit_info
	float impulse_on_exit = 1000.0f;
	bool preserve_entry_offset = false;
	// END GEN INTROSPECTOR
};

struct editor_point_marker_node_editable {
	// GEN INTROSPECTOR struct editor_point_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;

	vec2 pos;
	real32 rotation = 0.0f;

	editor_portal_exit_info as_portal_exit;
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
