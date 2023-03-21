#pragma once
#include <algorithm>

#include "augs/math/vec2.h"
#include "augs/math/transform.h"

struct snapping_grid {
	static auto of_one() {
		snapping_grid g;
		g.unit_pixels = 1;
		g.unit_degrees = 1;
		return g;
	}

	// GEN INTROSPECTOR struct snapping_grid
	int unit_pixels = 32;
	int unit_degrees = 15;
	// END GEN INTROSPECTOR

	void clamp_units(const int lower, const int upper) {
		unit_pixels = std::clamp(unit_pixels, lower, upper);
	}

	void increase_grid_size() {
		unit_pixels *= 2;
	}

	void decrease_grid_size() {
		unit_pixels /= 2;
	}

	/* transform snap(transform position) const; */

	vec2i get_snapping_corner(vec2 position) const;
	vec2 get_snapping_delta(ltrb position) const;

	int get_snapped(float degrees) const;
};
