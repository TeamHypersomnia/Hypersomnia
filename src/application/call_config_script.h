#pragma once
#include "application/game_window.h"

namespace sol {
	class state;
}

void call_config_script(
	sol::state& lua,
	const std::string& config_lua_path, 
	const std::string& config_local_lua_path
);