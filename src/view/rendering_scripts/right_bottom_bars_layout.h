#pragma once
#include <algorithm>
#include "view/game_drawing_settings.h"
#include "view/rendering_scripts/minimap_layout.h"

inline int calc_right_bottom_bars_raise(const game_drawing_settings& drawing, const int screen_height) {
	return std::clamp(drawing.right_bottom_bars_raise, 0, screen_height / 2);
}

inline int calc_right_bottom_bars_shift(const game_drawing_settings& drawing, const int screen_height) {
	return
		::calc_minimap_corner_reservation(drawing.minimap, hud_corner_type::RIGHT_BOTTOM)
		+ ::calc_right_bottom_bars_raise(drawing, screen_height)
	;
}
