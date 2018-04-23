#pragma once
#include <vector>
#include <chrono>

#include "augs/graphics/rgba.h"
#include "augs/filesystem/path.h"

struct neon_map_input {
	// GEN INTROSPECTOR struct neon_map_input
	float standard_deviation = 0.f;
	unsigned radius_towards_x_axis = 0;
	unsigned radius_towards_y_axis = 0;
	float amplification = 0.f;
	float alpha_multiplier = 1.f;

	std::vector<rgba> light_colors;
	// END GEN INTROSPECTOR
};

struct neon_map_stamp {
	// GEN INTROSPECTOR struct neon_map_stamp
	neon_map_input input;
	std::chrono::system_clock::time_point last_write_time_of_source;
	// END GEN INTROSPECTOR
};

void regenerate_neon_map(
	const augs::path_type& input_image_path,
	const augs::path_type& output_image_path,
	const neon_map_input in,
	const bool force_regenerate
);