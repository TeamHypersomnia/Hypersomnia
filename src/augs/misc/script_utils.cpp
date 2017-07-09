#include "script_utils.h"

namespace augs {
	sol::protected_function_result lua_error_callback(lua_State* s, sol::protected_function_result pfr) {
		LOG(pfr.operator std::string());
		ensure(pfr.valid());
		return pfr;
	}

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
		
		lua.script_file("scripts/utils.lua", augs::lua_error_callback);

		return lua;
	}

	sol::state& get_thread_local_lua_state() {
		thread_local sol::state lua = create_lua_state();
		return lua;
	}
}