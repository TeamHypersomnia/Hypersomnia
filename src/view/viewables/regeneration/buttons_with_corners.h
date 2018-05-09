#pragma once
#include "augs/graphics/rgba.h"
#include "augs/filesystem/path.h"

struct button_with_corners_input {
	// GEN INTROSPECTOR struct button_with_corners_input
	rgba border_color = white;
	rgba inside_color = white;

	unsigned lower_side = 0u;
	unsigned upper_side = 0u;

	unsigned inside_border_padding = 0u;
	// END GEN INTROSPECTOR
};

using button_with_corners_stamp = button_with_corners_input;

void regenerate_button_with_corners(
	const augs::path_type& output_image_path_template,
	const button_with_corners_input input,
	const bool force_regenerate
);

void create_and_save_button_with_corners(
	const augs::path_type& path_template,
	const button_with_corners_stamp
);