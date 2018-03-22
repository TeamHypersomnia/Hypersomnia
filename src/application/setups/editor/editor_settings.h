#pragma once
#include "augs/filesystem/path.h"
#include "augs/graphics/rgba.h"

struct editor_autosave_settings {
	// GEN INTROSPECTOR struct editor_autosave_settings
	bool enabled = true;
	bool on_lost_focus = true;
	double once_every_min = 1.0;
	// END GEN INTROSPECTOR

	bool operator!=(const editor_autosave_settings b) const {
		return once_every_min != b.once_every_min || enabled != b.enabled;
	}
};

struct editor_go_to_settings {
	// GEN INTROSPECTOR struct editor_go_to_settings
	unsigned dialog_width = 400;
	unsigned num_lines = 15;
	// END GEN INTROSPECTOR
};

struct editor_entity_selector_settings {
	// GEN INTROSPECTOR struct editor_entity_selector_settings
	rgba held_color = { 65, 131, 196, 160 };
	rgba selected_color = { 65, 131, 196, 120 };
	rgba hovered_color = { 255, 255, 255, 80 };
	// END GEN INTROSPECTOR
};

struct editor_camera_settings {
	// GEN INTROSPECTOR struct editor_camera_settings
	float panning_speed = 1.f;
	// END GEN INTROSPECTOR
};

struct editor_settings {
	// GEN INTROSPECTOR struct editor_settings
	editor_autosave_settings autosave;
	
#if TODO
	unsigned remember_last_n_commands = 200;
#endif

	editor_camera_settings camera;
	editor_go_to_settings go_to;
	editor_entity_selector_settings entity_selector;

	rgba controlled_entity_color = { 255, 255, 0, 120 };
	rgba matched_entity_color = { 0, 255, 0, 80 };

	rgba rectangular_selection_color = { 65, 131, 196, 60 };
	rgba rectangular_selection_border_color = { 65, 131, 196, 120 };

	rgba different_values_frame_bg = { 115, 73, 0, 255 };
	rgba different_values_frame_hovered_bg = { 158, 122, 0, 255 };
	rgba different_values_frame_active_bg = { 168, 158, 0, 255 };
	// END GEN INTROSPECTOR
};