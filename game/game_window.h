#pragma once
#include "augs/window_framework/window.h"
#include "augs/scripting/lua_state_wrapper.h"
#include "augs/misc/machine_entropy.h"

#include <thread>
#include <mutex>

class game_window {
	augs::lua_state_wrapper lua;
	std::mutex lua_mutex;
	window::glwindow window;

public:
	game_window();

	rects::wh<int> get_screen_rect();
	void swap_buffers();

	bool clear_window_inputs_once = true;

	volatile bool should_quit = false;

	decltype(machine_entropy::local) collect_entropy();

	void call_window_script(std::string filename);

	double get_config_number(std::string field);
	std::string get_config_string(std::string field);

	enum class launch_mode {
		INVALID,

		LOCAL,
		LOCAL_DETERMINISM_TEST,

		ONLY_CLIENT,
		ONLY_SERVER,
		
		CLIENT_AND_SERVER,
		TWO_CLIENTS_AND_SERVER
	} get_launch_mode();
};