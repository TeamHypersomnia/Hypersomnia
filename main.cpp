#pragma once
#include "game_framework/resources/lua_state_wrapper.h"
#include "game_framework/game_framework.h"

int main() {
	framework::init();

	resources::lua_state_wrapper lua_state;
	lua_state.bind_whole_engine();

	lua_state.dofile("init.lua");

	framework::deinit();
	return 0;
}