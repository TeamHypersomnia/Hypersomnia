#pragma once
#include "augs/window_framework/window.h"
#include "augs/misc/machine_entropy.h"

#include "config_lua_table.h"

class game_window {
	augs::window::glwindow window;
	friend class config_lua_table;

public:
	vec2i get_screen_size();
	void swap_buffers();

	bool clear_window_inputs_once = true;

	decltype(augs::machine_entropy::local) collect_entropy(const bool enable_cursor_clipping);
};