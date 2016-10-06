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

	template<class T>
	void get_config_value(const std::string& field, T& into) {
		std::unique_lock<std::mutex> lock(lua_mutex);

		into = luabind::object_cast<T>(luabind::globals(lua.raw)["config_table"][field]);
	}

	void get_config_value(const std::string& field, bool& into);

public:
	game_window();

	rects::wh<int> get_screen_rect();
	void swap_buffers();

	bool clear_window_inputs_once = true;

	config_values config;

	decltype(machine_entropy::local) collect_entropy();

	void call_window_script(const std::string& filename, const std::string& alternative_filename);

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