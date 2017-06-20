#pragma once
#include "augs/graphics/rgba.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/padding_byte.h"

struct button_with_corners_input {
	// GEN INTROSPECTOR struct button_with_corners_input
	rgba border_color;
	rgba inside_color;

	unsigned lower_side = 0u;
	unsigned upper_side = 0u;

	unsigned inside_border_padding = 0u;
	bool make_lb_complement = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR
};

using button_with_corners_stamp = button_with_corners_input;

void regenerate_button_with_corners(
	const std::string& output_image_path_template,
	const button_with_corners_input input,
	const bool force_regenerate
);

void create_and_save_button_with_corners(
	const std::string& filename_template,
	const button_with_corners_stamp
);