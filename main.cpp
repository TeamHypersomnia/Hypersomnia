#pragma once
#include "utilities/lua_state_wrapper.h"
#include "game_framework/game_framework.h"

#include "gui\hypersomnia_gui.h"
#include "augmentations.h"

int main() {
	framework::init();

	augs::lua_state_wrapper lua_state;
	framework::bind_whole_engine(lua_state);
	hypersomnia_gui::bind(lua_state);

	lua_state.dofile("init.lua");

	framework::deinit();
	return 0;
}