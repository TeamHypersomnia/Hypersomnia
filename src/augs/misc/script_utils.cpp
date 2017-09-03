#include "script_utils.h"

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

		const auto utils_path = "scripts/utils.lua";
		
		auto pfr = lua.do_file(utils_path);

		if (!pfr.valid()) {
			LOG("Fatal error: there was a problem building %x:\n%x", utils_path);
			press_any_key();
			cleanup_proc();
		}

		return lua;
	}

	sol::state& get_thread_local_lua_state() {
		thread_local sol::state lua = create_lua_state();
		return lua;
	}
}