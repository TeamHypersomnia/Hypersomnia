#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "game_window.h"
#include "game/bindings/bind_game_and_augs.h"
#include "augs/log.h"

game_window::game_window() {
	bind_game_and_augs(lua);
}

rects::wh<int> game_window::get_screen_rect() {
	std::unique_lock<std::mutex> lock(lua_mutex);
	return window.get_screen_rect();
}

void game_window::swap_buffers() {
	window.swap_buffers();
}

void game_window::call_window_script(const std::string& filename) {
	{
		std::unique_lock<std::mutex> lock(lua_mutex);

		lua.global_ptr("global_gl_window", &window);

		try {
			if (!lua.dofile(filename))
				lua.debug_response();
		}
		catch (char* e) {
			LOG("Exception thrown! %x", e);
			lua.debug_response();
		}
		catch (...) {
			LOG("Exception thrown!");
			lua.debug_response();
		}

		window.gl.initialize();
	}

	config.get_values(*this);
}

double game_window::get_config_number(const std::string& field) {
	std::unique_lock<std::mutex> lock(lua_mutex);

	return luabind::object_cast<double>(luabind::globals(lua.raw)["config_table"][field]);
}

bool game_window::get_flag(const std::string& field) {
	return get_config_number(field) > 0.0;
}

game_window::launch_mode game_window::get_launch_mode() {
	switch (static_cast<int>(config.launch_mode)) {
	case 0: return launch_mode::LOCAL; break;
	case 1: return launch_mode::LOCAL_DETERMINISM_TEST; break;
	case 2: return launch_mode::ONLY_CLIENT; break;
	case 3: return launch_mode::ONLY_SERVER; break;
	case 4: return launch_mode::CLIENT_AND_SERVER; break;
	case 5: return launch_mode::TWO_CLIENTS_AND_SERVER; break;
	default: return launch_mode::INVALID; break;
	}
}

std::string game_window::get_config_string(const std::string& field) {
	std::unique_lock<std::mutex> lock(lua_mutex);

	return luabind::object_cast<std::string>(luabind::globals(lua.raw)["config_table"][field]);
}

decltype(machine_entropy::local) game_window::collect_entropy() {
	auto result = window.poll_events(!config.debug_disable_cursor_clipping);

	if (clear_window_inputs_once) {
		result.clear();
		clear_window_inputs_once = false;
	}
	
	return result;
}