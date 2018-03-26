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

	// GEN INTROSPECTOR struct grid
	unsigned unit_pixels = 16;
	unsigned unit_degrees = 15;
	// END GEN INTROSPECTOR

	void clamp_units(const unsigned lower, const unsigned upper) {
		unit_pixels = std::clamp(unit_pixels, lower, upper);
	}

	void increase_grid_size() {
		unit_pixels *= 2;
	}

	void decrease_grid_size() {
		unit_pixels /= 2;
	}

	transform snap(transform position) const;
	vec2 snap(vec2 position) const;
	float snap(float rotation) const;
};
