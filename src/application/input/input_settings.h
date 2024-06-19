#pragma once
#include "view/game_gui_input_settings.h"
#include "game/per_character_input_settings.h"

struct input_settings {
	// GEN INTROSPECTOR struct input_settings
	per_character_input_settings character;
	bool swap_mouse_buttons_in_akimbo = true;
	game_gui_input_settings game_gui;
	// END GEN INTROSPECTOR

	bool operator==(const input_settings& b) const = default;
};
