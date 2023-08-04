#pragma once
#include "application/setups/editor/resources/editor_particle_effect.h"
#include "application/setups/editor/resources/editor_sound_effect.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "game/components/marker_component.h"
#include "game/detail/view_input/continuous_rings_input.h"

#include "application/setups/editor/nodes/editor_color_preset.h"

#include "game/detail/sentience_shake.h"
#include "game/enums/portal_enums.h"

struct editor_point_marker_node;

struct editor_filter_flags {
	// GEN INTROSPECTOR struct editor_filter_flags
	bool characters = true;
	bool character_weapons = true;
	bool bullets = true;
	bool flying_explosives = true;
	bool flying_melees = true;
	bool lying_items = true;
	bool shells = true;
	bool obstacles = true;
	// END GEN INTROSPECTOR

	bool operator==(const editor_filter_flags& b) const = default;

	uint16_t get_mask_bits() const;
};

struct editor_portal_info {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct editor_portal_info
	bool disable_all_entry_effects = false;
	bool disable_all_exit_effects = false;

	editor_color_preset color_preset = editor_color_preset::CYAN;

	bool auto_scale_pitches = true;
	bool trampoline_like = false;
	bool ignore_airborne_characters = false;
	bool ignore_walking_characters = false;

	float enter_time_ms = 500.0f;
	float travel_time_ms = 500.0f;
	float exit_cooldown_ms = 200.0f;

	augs::maybe<force_field_def> force_field = augs::maybe<force_field_def>(force_field_def(), false);
	augs::maybe<hazard_def> hazard = augs::maybe<hazard_def>(hazard_def(), false);

	editor_filter_flags reacts_to;
	per_actual_faction<bool> reacts_to_factions = { true, true, true };

	float begin_entering_highlight_ms = 1000.0f;
	rgba_channel decrease_opacity_to = 0;
	float exit_highlight_ms = 700.0f;

	augs::maybe<continuous_rings_input> rings_effect = continuous_rings_input();
	float light_size_mult = 2.05f;
	rgba light_color = rgba(0, 255, 255, 150);

	sentience_shake enter_shake = { 1.0f, 1500.0f };
	sentience_shake exit_shake = { 1.0f, 1500.0f };

	editor_sound_effect ambience_sound;
	float ambience_sound_distance_mult = 2.0f;

	editor_particle_effect ambience_particles;

	editor_sound_effect begin_entering_sound;
	editor_sound_effect enter_sound;
	editor_sound_effect exit_sound;

	editor_particle_effect begin_entering_particles;
	editor_particle_effect enter_particles;
	editor_particle_effect exit_particles;

	editor_typed_node_id<editor_area_marker_node> portal_exit;
	portal_exit_impulses exit_impulses;
	portal_exit_direction exit_direction = portal_exit_direction::PORTAL_DIRECTION;
	portal_exit_position exit_position = portal_exit_position::PORTAL_CENTER_PLUS_ENTERING_OFFSET;

	augs::maybe<std::string> context_tip;
	// END GEN INTROSPECTOR

	bool operator==(const editor_portal_info&) const = default;

	void apply(editor_color_preset);
	rgba get_icon_color() const;

	editor_portal_info() {
		apply(editor_color_preset::CYAN);
	}
};

struct editor_point_marker_node_editable {
	// GEN INTROSPECTOR struct editor_point_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;

	vec2 pos;
	real32 rotation = 0.0f;
	// END GEN INTROSPECTOR

	bool operator==(const editor_point_marker_node_editable&) const = default;
};

struct editor_area_marker_node_editable {
	// GEN INTROSPECTOR struct editor_area_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;
	marker_shape_type shape = marker_shape_type::BOX;

	vec2 pos;
	real32 rotation = 0.0f;
	vec2i size = { 256, 256 };

	editor_portal_info as_portal;
	// END GEN INTROSPECTOR

	bool operator==(const editor_area_marker_node_editable&) const = default;
};
