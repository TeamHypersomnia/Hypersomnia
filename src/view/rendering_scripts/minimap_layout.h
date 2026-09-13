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

	const auto lt = [&]() {
		switch (settings.position) {
			case hud_corner_type::LEFT_TOP:
				return vec2i(margin, margin);
			case hud_corner_type::RIGHT_TOP:
				return vec2i(screen_size.x - margin - size.x, margin);
			case hud_corner_type::LEFT_BOTTOM:
				return vec2i(margin, screen_size.y - margin - size.y);
			case hud_corner_type::RIGHT_BOTTOM:
			default:
				return vec2i(screen_size.x - margin - size.x, screen_size.y - margin - size.y);
		}
	}();

	return ltrbi(lt, size);
}
