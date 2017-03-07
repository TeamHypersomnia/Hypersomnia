#pragma once
#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/padding_byte.h"

struct button_with_corners_stamp {
	rgba border_color;
	rgba inside_color;

	unsigned lower_side = 0u;
	unsigned upper_side = 0u;

	unsigned inside_border_padding = 0u;
	bool make_lb_complement = false;
	std::array<padding_byte, 3> pad;
};

void regenerate_buttons_with_corners(
	const bool force_regenerate
);

void create_and_save_button_with_corners(
	const std::string& filename_template,
	const button_with_corners_stamp
);