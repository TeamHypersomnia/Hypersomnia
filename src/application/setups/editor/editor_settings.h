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

struct editor_settings {
	// GEN INTROSPECTOR struct editor_settings
	editor_autosave_settings autosave;
	float camera_panning_speed = 1.f;
	
	unsigned remember_last_n_commands = 200;

	unsigned go_to_dialog_width = 400;
	unsigned lines_in_go_to_dialogs = 15;
	
	rgba controlled_entity_color = { 255, 255, 0, 120 };
	rgba held_entity_color = { 65, 131, 196, 160 };
	rgba selected_entity_color = { 65, 131, 196, 120 };
	rgba hovered_entity_color = { 255, 255, 255, 80 };

	rgba matched_entity_color = { 0, 255, 0, 80 };

	rgba rectangular_selection_color = { 65, 131, 196, 60 };
	rgba rectangular_selection_border_color = { 65, 131, 196, 120 };
	// END GEN INTROSPECTOR
};