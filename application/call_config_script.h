#pragma once
#include "application/game_window.h"

namespace augs {
	class lua_state_raii;
}

void call_config_script(
	augs::lua_state_raii& lua, 
	const std::string& config_lua_path, 
	const std::string& config_override_lua_path
);

void call_window_script(
	augs::lua_state_raii& lua, 
	game_window& window, 
	const std::string& window_lua_path
);