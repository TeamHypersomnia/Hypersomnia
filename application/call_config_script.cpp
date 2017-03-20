#include "call_config_script.h"
#include "augs/filesystem/file.h"
#include "augs/scripting/lua_state_raii.h"

#include <luabind/luabind.hpp>

void call_window_script(augs::lua_state_raii& lua, game_window& window, const std::string& filename) {
	lua.global_ptr("global_gl_window", &window.window);
	lua.dofile_and_report_errors(filename);
}

void call_config_script(augs::lua_state_raii& lua, const std::string& filename, const std::string& alternative_filename) {
	std::string used_filename = filename;

	if (augs::file_exists(alternative_filename)) {
		used_filename = alternative_filename;
	}

	lua.dofile_and_report_errors(used_filename);
}
