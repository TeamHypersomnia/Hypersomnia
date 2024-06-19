#pragma once
#include "augs/graphics/rgba.h"

struct grid_render_settings {
	// GEN INTROSPECTOR struct grid_render_settings
	unsigned maximum_power_of_two = 12;
	std::array<rgba, 10> line_colors = { red, pink, orange, yellow, green, cyan, white, violet, gray, gray };
	float alpha_multiplier = 0.5f;
	unsigned hide_grids_smaller_than = 16;
	// END GEN INTROSPECTOR

	bool operator==(const grid_render_settings& b) const = default;

	int get_maximum_unit() const {
		return 1 << maximum_power_of_two;
	}
};
