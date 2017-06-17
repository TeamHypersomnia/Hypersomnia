#pragma once
#include <array>
#include "augs/padding_byte.h"
#include "augs/graphics/rgba.h"

struct hotbar_settings {
	// GEN INTROSPECTOR struct hotbar_settings
	bool increase_inside_alpha_when_selected = false;
	bool colorize_inside_when_selected = true;
	pad_bytes<2> pad;

	rgba primary_selected_color = rgba(0, 255, 255, 255);
	rgba secondary_selected_color = rgba(86, 156, 214, 255);
	// END GEN INTROSPECTOR
};