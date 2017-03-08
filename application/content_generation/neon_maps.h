#pragma once
#include <vector>
#include <chrono>

#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/templates/maybe_const.h"

struct neon_map_stamp {
	// GEN INTROSPECTOR neon_map_stamp
	float standard_deviation = 0.f;
	unsigned radius_towards_x_axis = 0xdeadbeef;
	unsigned radius_towards_y_axis = 0xdeadbeef;
	float amplification = 0.f;
	float alpha_multiplier = 1.f;
	std::chrono::system_clock::time_point last_write_time_of_source;

	std::vector<rgba> light_colors;
	// END GEN INTROSPECTOR
};

void regenerate_neon_maps(
	const bool force_regenerate
);