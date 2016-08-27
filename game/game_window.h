#pragma once
#include "augs/window_framework/window.h"
#include "augs/scripting/lua_state_wrapper.h"
#include "augs/misc/machine_entropy.h"

class game_window {
public:
	game_window();

	augs::lua_state_wrapper lua;
	window::glwindow window;
	bool clear_window_inputs_once = true;

	decltype(machine_entropy::local) collect_entropy();

	void call_window_script(std::string filename);

	double get_config_number(std::string field);
	std::string get_config_string(std::string field);

	enum class launch_mode {
		INVALID,

		LOCAL,

		ONLY_CLIENT,
		ONLY_SERVER,
		
		CLIENT_AND_SERVER
	} get_launch_mode();
};