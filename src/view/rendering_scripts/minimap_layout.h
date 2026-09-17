#pragma once
#include "augs/math/rects.h"
#include "view/game_drawing_settings.h"

constexpr int minimap_screen_margin_v = 10;

inline ltrbi calc_minimap_rect(
	const minimap_settings& settings,
	const vec2i screen_size
) {
	const auto size = vec2i::square(settings.size);
	const auto margin = minimap_screen_margin_v;

	/*
		Runtime HUD (e.g. the tutorial's bottom progress bar)
		pushes the minimap up when it sits in a bottom corner.
	*/
	const auto bottom_y = screen_size.y - margin - size.y - settings.extra_bottom_margin;

	const auto lt = [&]() {
		switch (settings.position) {
			case hud_corner_type::LEFT_TOP:
				return vec2i(margin, margin);
			case hud_corner_type::RIGHT_TOP:
				return vec2i(screen_size.x - margin - size.x, margin);
			case hud_corner_type::LEFT_BOTTOM:
				return vec2i(margin, bottom_y);
			case hud_corner_type::RIGHT_BOTTOM:
			default:
				return vec2i(screen_size.x - margin - size.x, bottom_y);
		}
	}();

	return ltrbi(lt, size);
}
