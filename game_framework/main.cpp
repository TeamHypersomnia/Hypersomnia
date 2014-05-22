#pragma once
#include "stdafx.h"

#include "resources/scriptable_info.h"
#include "resources/lua_state_wrapper.h"

#include "game_framework.h"

int main() {
	framework::init();

	resources::lua_state_wrapper lua_state;
	lua_state.bind_whole_engine();

	lua_state.dofile("config.lua");

	augs::window::glwindow gl;
	gl.create(lua_state, augs::rects::wh<int>(100, 100));
	gl.set_show(gl.SHOW);
	gl.vsync(0);
	augs::window::cursor(false);
	 
	framework::set_current_window(gl);

	lua_state.dofile("init.lua");
	framework::deinit();
	return 0;
}
