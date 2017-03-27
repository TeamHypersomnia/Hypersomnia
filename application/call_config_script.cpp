#include "call_config_script.h"
#include "augs/filesystem/file.h"
#include "augs/scripting/lua_state_raii.h"

#include <luabind/luabind.hpp>

void call_window_script(
	augs::lua_state_raii& lua,
	game_window& window,
	const std::string& window_lua_path
) {
	lua.global_ptr("global_gl_window", &window.window);
	lua.dofile_and_report_errors(window_lua_path);
}

void call_config_script(
	augs::lua_state_raii& lua,
	const std::string& config_lua_path,
	const std::string& config_override_lua_path
) {
	lua.dofile_and_report_errors(config_lua_path);

	if (augs::file_exists(config_override_lua_path)) {
		lua.dofile_and_report_errors(config_override_lua_path);
	}
}
