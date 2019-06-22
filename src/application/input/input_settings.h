#pragma once
#include "view/game_gui_input_settings.h"

struct input_settings {
	// GEN INTROSPECTOR struct input_settings
	vec2 mouse_sensitivity = vec2(3.f, 3.f);
	bool swap_mouse_buttons_in_akimbo = true;
	game_gui_input_settings game_gui;
	// END GEN INTROSPECTOR
};
