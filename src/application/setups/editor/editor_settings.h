#pragma once
#include "augs/filesystem/path.h"
#include "augs/graphics/rgba.h"

#include "augs/drawing/grid_render_settings.h"

enum class editor_autosave_load_option {
	// GEN INTROSPECTOR enum class editor_autosave_load_option
	AUTOSAVED_VERSION,
	LAST_SAVED_VERSION,
	COUNT
	// END GEN INTROSPECTOR
};

struct editor_autosave_settings {
	// GEN INTROSPECTOR struct editor_autosave_settings
	bool on_lost_focus = true;

	bool periodically = true;
	double once_every_min = 1.0;
	editor_autosave_load_option if_loaded_autosave_show = editor_autosave_load_option::AUTOSAVED_VERSION;
	bool alert_when_loaded_autosave = true;
	// END GEN INTROSPECTOR

	bool interval_changed(const editor_autosave_settings b) const {
		return once_every_min != b.once_every_min || periodically != b.periodically;
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
	rgba hovered_dashed_line_color = { 255, 255, 255, 140 };
	// END GEN INTROSPECTOR
};

struct editor_camera_settings {
	// GEN INTROSPECTOR struct editor_camera_settings
	float panning_speed = 1.f;
	// END GEN INTROSPECTOR
};

struct editor_grid_settings {
	// GEN INTROSPECTOR struct editor_grid_settings
	grid_render_settings render;
	// END GEN INTROSPECTOR
};

struct editor_action_notification_settings {
	// GEN INTROSPECTOR struct editor_action_notification_settings
	bool enabled = true;

	rgba bg_color = { 0, 0, 0, 180 };
	rgba bg_border_color = { 255, 255, 255, 15 };

	unsigned max_width = 300;
	unsigned show_for_ms = 3000;
	vec2i text_padding = vec2i(5, 5);
	vec2i offset = vec2i(80, 80);
	// END GEN INTROSPECTOR
};

struct editor_settings {
	// GEN INTROSPECTOR struct editor_settings
	editor_autosave_settings autosave;
	bool warp_cursor_when_moving_nodes = false;
	bool keep_source_nodes_selected_on_mirroring = false;

	editor_grid_settings grid;

	editor_camera_settings camera;
	editor_entity_selector_settings entity_selector;

	rgba matched_entity_color = { 0, 255, 0, 80 };

	rgba rectangular_selection_color = { 65, 131, 196, 60 };
	rgba rectangular_selection_border_color = { 65, 131, 196, 120 };

	editor_action_notification_settings action_notification;
	// END GEN INTROSPECTOR
};