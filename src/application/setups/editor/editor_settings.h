#pragma once
#include "augs/filesystem/path.h"

struct editor_autosave_settings {
	// GEN INTROSPECTOR struct editor_autosave_settings
	double once_every_min = 1.0;
	bool enabled = true;
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

	rgba controlled_entity_color = { 255, 255, 0, 120 };
	rgba held_entity_color = { 65, 131, 196, 160 };
	rgba selected_entity_color = { 65, 131, 196, 120 };
	rgba hovered_entity_color = { 255, 255, 255, 80 };

	rgba rectangular_selection_color = { 65, 131, 196, 60 };
	rgba rectangular_selection_border_color = { 65, 131, 196, 120 };
	// END GEN INTROSPECTOR
};