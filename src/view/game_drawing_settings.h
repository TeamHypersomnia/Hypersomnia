#pragma once
#include "augs/pad_bytes.h"
#include "augs/templates/maybe.h"
#include "augs/graphics/rgba.h"

struct game_drawing_settings {
	// GEN INTROSPECTOR struct game_drawing_settings
	bool draw_crosshairs = true;
	bool draw_weapon_laser = true;
	bool draw_aabb_highlighter = true;
	augs::maybe<float> draw_area_markers = augs::maybe<float>(0.5f, true);
	bool draw_enemy_hud = false;

	bool fog_of_war = true;
	rgba fog_of_war_color = rgba(255, 255, 255, 30);
	// END GEN INTROSPECTOR
};