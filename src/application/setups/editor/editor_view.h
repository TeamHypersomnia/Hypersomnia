#pragma once
#include <unordered_set>
#include <optional>

#include "augs/misc/time_utils.h"
#include "augs/math/camera_cone.h"
#include "augs/math/snapping_grid.h"

#include "augs/templates/container_templates.h"

#include "game/transcendental/entity_id.h"

#include "application/setups/editor/editor_selection_groups.h"
#include "application/setups/editor/gui/editor_rect_select_type.h"

struct editor_folder_meta {
	// GEN INTROSPECTOR struct editor_folder_meta
	augs::date_time timestamp;
	// END GEN INTROSPECTOR
};

using current_selections_type = std::unordered_set<entity_id>;

struct editor_view {
	// GEN INTROSPECTOR struct editor_view
	editor_folder_meta meta;
	snapping_grid grid;
	bool show_grid = true;
	bool snapping_enabled = true;
	bool ignore_groups = false;
	editor_rect_select_type rect_select_mode = editor_rect_select_type::SAME_FLAVOUR;

	editor_selection_groups selection_groups;
	current_selections_type selected_entities;

	std::optional<camera_cone> panned_camera;
	// END GEN INTROSPECTOR

	void reset_zoom();
	void reset_zoom_at(vec2);
	void center_at(vec2);

	void toggle_snapping();
	void toggle_ignore_groups();
	void toggle_grid();
	void toggle_flavour_rect_selection();
};
