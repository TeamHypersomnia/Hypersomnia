#pragma once
#include "application/game_window.h"

namespace augs {
	class lua_state_raii;
}

void call_config_script(augs::lua_state_raii& lua, const std::string& filename, const std::string& alternative_filename);
void call_window_script(augs::lua_state_raii& lua, game_window&, const std::string& filename);