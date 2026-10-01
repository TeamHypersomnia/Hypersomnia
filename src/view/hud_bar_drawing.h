#pragma once
#include "augs/math/rects.h"
#include "augs/drawing/drawing.h"
#include "view/necessary_resources.h"
#include "view/hud_bar_state.h"

namespace augs {
	struct baked_font;
}

/*
	What every bar of a given HUD gets drawn with.
	highlight_base_duration_secs is the duration of the white highlight
	of a damaged character's health bar - the bars' flashes are timed after it.
*/

struct hud_bar_draw_input {
	const augs::drawer_with_default output;
	const necessary_images_in_atlas_map& necessary_images;
	const double total_secs;
	const float highlight_base_duration_secs;
	const augs::baked_font& font;
};

/*
	Draws a complete bar: the black frame, the darkened background,
	the fill with its bright border, the outline at the fill's moving edge,
	the flowing particles, the white flash on increase
	and the appearance's optional label.

	bordered_rect is the outer rect of the bright border.
*/

void draw_hud_bar(
	const hud_bar_draw_input& in,
	hud_bar_state& state,
	const hud_bar_appearance& appearance,
	ltrb bordered_rect,
	float ratio
);
