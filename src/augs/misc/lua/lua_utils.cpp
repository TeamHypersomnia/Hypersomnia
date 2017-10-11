#include "augs/log.h"
#include "augs/ensure.h"

#include "augs/misc/lua/lua_utils.h"

namespace augs {
	sol::state create_lua_state() {
		sol::state lua;

		lua.open_libraries(
			sol::lib::base,
			sol::lib::package,
			sol::lib::string,
			sol::lib::os,
			sol::lib::math,
			sol::lib::table,
			sol::lib::debug,
			sol::lib::bit32,
			sol::lib::io,
			sol::lib::utf8
		);
		
		lua["LOG"] = [](const std::string content) { 
			LOG(content); 
		};

		lua["ensure"] = [](
			const bool condition, 
			const std::string message
		) { 
			if (!condition) { 
				LOG(message); 
				ensure(false); 
			} 
		};

		if (
			const auto utils_path = "scripts/utils.lua";
			!lua.do_file(utils_path).valid()
		) {
			throw lua_state_creation_error(
				"Failed to build %x", utils_path
			);
		}

		return lua;
	}
}