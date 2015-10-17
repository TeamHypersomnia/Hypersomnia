#pragma once
#include "stdafx.h"

#include "utilities/lua_state_wrapper.h"
#include "game_framework/game_framework.h"

#include "gui\hypersomnia_gui.h"
#include "augmentations.h"

#include "game/utilities.h"
#include "game/bindings.h"

#include <gtest\gtest.h>

int main(int argc, char** argv) {
	framework::init();

	augs::lua_state_wrapper lua_state;
	framework::bind_whole_engine(lua_state);
	hypersomnia_gui::bind(lua_state);
	bind_hypersomnia_implementations(lua_state);

	//::testing::InitGoogleTest(&argc, argv);
	//RUN_ALL_TESTS();

	lua_state.dofile("init.lua");

	framework::deinit();
	return 0;
}