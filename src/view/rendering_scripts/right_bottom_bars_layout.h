#pragma once
#include "view/game_drawing_settings.h"
#include "view/rendering_scripts/minimap_layout.h"

inline int calc_right_bottom_bars_shift(const game_drawing_settings& drawing) {
	return
		::calc_minimap_corner_reservation(drawing.minimap, hud_corner_type::RIGHT_BOTTOM)
		+ drawing.right_bottom_bars_raise
	;
}
