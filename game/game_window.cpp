#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "game_window.h"
#include "game/bindings/bind_game_and_augs.h"
#include "augs/log.h"

game_window::game_window() {
	bind_game_and_augs(lua);
}

void game_window::call_window_script(const std::string filename) {
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

double game_window::get_config_number(std::string field) {
	return luabind::object_cast<double>(luabind::globals(lua.raw)["config_table"][field]);
}

std::string game_window::get_config_string(std::string field) {
	return luabind::object_cast<std::string>(luabind::globals(lua.raw)["config_table"][field]);
}

decltype(machine_entropy::local) game_window::collect_entropy() {
	auto result = window.poll_events();

	if (clear_window_inputs_once) {
		result.clear();
		clear_window_inputs_once = false;
	}
	
	return result;
}