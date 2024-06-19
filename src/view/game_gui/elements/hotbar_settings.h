#pragma once
#include <array>
#include "augs/pad_bytes.h"
#include "augs/graphics/rgba.h"

struct hotbar_settings {
	// GEN INTROSPECTOR struct hotbar_settings
	bool increase_inside_alpha_when_selected = false;
	bool colorize_inside_when_selected = true;

	bool hide_unassigned_hotbar_buttons = true;
	bool autocollapse_hotbar_buttons = true;

	rgba primary_selected_color = rgba(0, 255, 255, 255);
	rgba secondary_selected_color = rgba(86, 156, 214, 255);
	// END GEN INTROSPECTOR

	bool operator==(const hotbar_settings& b) const = default;
};