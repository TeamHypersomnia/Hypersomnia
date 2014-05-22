#pragma once
#include "stdafx.h"
#include <gtest\gtest.h>
#include "augmentations.h"
#include "window_framework/window.h"

#include "resources/scriptable_info.h"

#include "world_instance.h"

using namespace augs;
using namespace entity_system;

window::glwindow* global_window = nullptr;

int main() {
	augs::init();	 
	resources::lua_state_wrapper lua_state;
	script_system::generate_lua_state(lua_state);

	lua_gc(lua_state, LUA_GCCOLLECT, 0);
	resources::script::script_reloader.report_errors = &std::cout;

	resources::script config_script(lua_state);
	config_script.associate_filename("config.lua");
	config_script.call();

	window::glwindow gl;
	gl.create(lua_state, rects::wh<int>(100, 100));
	gl.set_show(gl.SHOW);
	gl.vsync(0);
	window::cursor(false); 
	 
	global_window = &gl;

	script_system::global(lua_state, "window", *global_window);
	script_system::global(lua_state, "script_reloader", resources::script::script_reloader);
	//world_instance instance;
	 
	resources::script init_script(lua_state);
	 
	init_script.associate_filename("init.lua");
 	init_script.call(); 

	lua_gc(lua_state, LUA_GCCOLLECT, 0);

	int argc = 0;
	::testing::InitGoogleTest(&argc, (wchar_t**)nullptr);

	::testing::FLAGS_gtest_catch_exceptions = false;
	::testing::FLAGS_gtest_break_on_failure = false;
	auto result = RUN_ALL_TESTS();

	using namespace augs::graphics;

	int should_quit = 0;
	while (!should_quit) {
		if (luabind::globals(lua_state)["augmentations_main_loop_callback"]) {
			try {
				should_quit = luabind::call_function<int>(luabind::globals(lua_state)["augmentations_main_loop_callback"]);
			}
			catch (std::exception compilation_error) {
				std::cout << compilation_error.what() << '\n';
			}
		}

		if (luabind::globals(lua_state)["call_once_after_loop"]) {
			try {
				luabind::call_function<void>(luabind::globals(lua_state)["call_once_after_loop"]);
			}
			catch (std::exception compilation_error) {
				std::cout << compilation_error.what() << '\n';
			}

			luabind::globals(lua_state)["call_once_after_loop"] = luabind::nil;
		}
		//instance.default_loop();
	} 

	augs::deinit();
	return 0;
}
