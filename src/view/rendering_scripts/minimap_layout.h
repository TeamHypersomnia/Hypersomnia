#pragma once
#include "augs/math/rects.h"
#include "view/game_drawing_settings.h"

constexpr int minimap_screen_margin_v = 10;

/*
	How much of the corner, from the screen's edge, the other HUD elements sharing it make room for:
	the minimap with its margin, and whatever is drawn right at its edge - see minimap_settings::extra_hud_space.
*/

inline int calc_minimap_corner_reservation(const minimap_settings& minimap, const hud_corner_type corner) {
	if (!minimap.occupies_corner(corner)) {
		return 0;
	}

	const bool at_bottom = corner == hud_corner_type::LEFT_BOTTOM || corner == hud_corner_type::RIGHT_BOTTOM;

	return 
		minimap.get_gameplay_appearance().size
		+ minimap_screen_margin_v
		+ minimap.extra_hud_space
		+ (at_bottom ? minimap.extra_bottom_margin : 0)
	;
}

inline ltrbi calc_minimap_rect(
	const minimap_settings& settings,
	const vec2i screen_size,
	const minimap_state_type state
) {
	const auto size = vec2i::square(settings.calc_size(state));
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

/*
	The scissor clipping what is drawn to the minimap - its border included, which lies outside of its rect.
	GL scissor origin is bottom-left.
*/

inline xywhi calc_minimap_scissor(
	const minimap_settings& settings,
	const vec2i screen_size,
	const minimap_state_type state
) {
	const auto minimap_rect = ::calc_minimap_rect(settings, screen_size, state);
	const auto expansion = settings.border_thickness;

	return {
		minimap_rect.l - expansion,
		screen_size.y - (minimap_rect.b + expansion),
		minimap_rect.w() + 2 * expansion,
		minimap_rect.h() + 2 * expansion
	};
}
