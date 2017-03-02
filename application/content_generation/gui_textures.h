#pragma once

struct button_with_corners_metadata {
	assets::texture_id inside_tex;

	rgba border_color;
	rgba inside_color;

	unsigned lower_side;
	unsigned upper_side;

	unsigned inside_border_padding;
	bool make_lb_complement;
};

void make_button_with_corners(
	const button_with_corners_metadata
);