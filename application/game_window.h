#pragma once
#include "augs/window_framework/window.h"
#include "augs/scripting/lua_state_wrapper.h"
#include "augs/misc/machine_entropy.h"

#include "config_values.h"

#include <thread>
#include <mutex>

class game_window {
	friend class config_values;

	augs::lua_state_wrapper lua;
	std::mutex lua_mutex;
	window::glwindow window;

	double get_config_number(const std::string field);
	bool get_flag(const std::string field);
	std::string get_config_string(const std::string field);

	template<class T>
	void get_config_value(T& into, const std::string field) {
		into = static_cast<T>(get_config_number(field));
	}

	void get_config_value(std::string& into, const std::string field) {
		into = get_config_string(field);
	}

	void get_config_value(bool& into, const std::string field) {
		into = get_flag(field);
	}

public:
	game_window();

	rects::wh<int> get_screen_rect();
	void swap_buffers();

	bool clear_window_inputs_once = true;

	config_values config;

	decltype(machine_entropy::local) collect_entropy();

	void call_window_script(const std::string filename);

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