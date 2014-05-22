#pragma once
#include "stdafx.h"
#include "window_framework/window.h"

#include "resources/scriptable_info.h"
#include "resources/lua_state_wrapper.h"

#include "game_framework.h"

augs::window::glwindow* global_window = nullptr;

int main() {
	framework::init();

	resources::lua_state_wrapper lua_state;
	lua_state.bind_whole_engine();

	lua_gc(lua_state, LUA_GCCOLLECT, 0);
	resources::script::script_reloader.report_errors = &std::cout;

	resources::script config_script(lua_state);
	config_script.associate_filename("config.lua");
	config_script.call();

	augs::window::glwindow gl;
	gl.create(lua_state, augs::rects::wh<int>(100, 100));
	gl.set_show(gl.SHOW);
	gl.vsync(0);
	augs::window::cursor(false);
	 
	global_window = &gl;

	lua_state.global("window", *global_window);
	lua_state.global("script_reloader", resources::script::script_reloader);
	 
	resources::script init_script(lua_state);
	 
	init_script.associate_filename("init.lua");
 	init_script.call(); 

	lua_gc(lua_state, LUA_GCCOLLECT, 0);

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
	} 

	framework::deinit();
	return 0;
}
