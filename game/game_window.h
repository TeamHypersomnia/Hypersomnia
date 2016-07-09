#pragma once
#include "window_framework/window.h"
#include "augs/scripting/lua_state_wrapper.h"
#include "augs/misc/machine_entropy.h"

class game_window {
public:
	game_window();

	augs::lua_state_wrapper lua;
	window::glwindow window;
	bool clear_window_inputs_once = true;

	machine_entropy collect_entropy();

	void call_window_script(std::string filename);
};