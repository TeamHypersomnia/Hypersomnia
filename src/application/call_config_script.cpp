#include "call_config_script.h"
#include "augs/filesystem/file.h"
#include "augs/misc/script_utils.h"

void call_config_script(
	sol::state& lua,
	const std::string& config_lua_path,
	const std::string& config_local_lua_path
) {
	std::string used_filename = config_lua_path;

	if (augs::file_exists(config_local_lua_path)) {
		used_filename = config_local_lua_path;
	}

	lua.script_file(used_filename, augs::lua_error_callback);
}
