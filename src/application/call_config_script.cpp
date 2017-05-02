#include "call_config_script.h"
#include "augs/filesystem/file.h"

#include <sol.hpp>

void call_config_script(
	sol::state& lua,
	const std::string& config_lua_path,
	const std::string& config_local_lua_path
) {
	std::string used_filename = config_lua_path;

	if (augs::file_exists(config_local_lua_path)) {
		used_filename = config_local_lua_path;
	}

	lua.script_file(used_filename, [](
		lua_State* L, 
		sol::protected_function_result pfr
	){
		LOG(pfr.operator std::string());
		ensure(pfr.valid());
		return pfr;
	});
}
