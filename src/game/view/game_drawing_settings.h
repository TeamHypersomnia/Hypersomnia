#pragma once
#include "augs/pad_bytes.h"

struct game_drawing_settings {
	// GEN INTROSPECTOR struct game_drawing_settings
	bool draw_character_gui = true;
	bool draw_crosshairs = true;
	bool draw_weapon_laser = true;
	pad_bytes<1> pad;
	// END GEN INTROSPECTOR
};