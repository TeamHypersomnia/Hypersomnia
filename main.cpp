#pragma once
#include "utilities/lua_state_wrapper.h"
#include "game_framework/game_framework.h"

#include "gui\hypersomnia_gui.h"
#include "augmentations.h"

#include "game/utilities.h"
#include "game/bindings.h"

#include <gtest\gtest.h>

#include <iostream>
#include <signal.h>
#include "script.h"

using namespace std;
using namespace augs;

void SignalHandler(int signal) { throw "Access violation!"; }

int main(int argc, char** argv) {
	framework::init();

	augs::lua_state_wrapper lua_state;
	framework::bind_whole_engine(lua_state);
	hypersomnia_gui::bind(lua_state);
	bind_hypersomnia_implementations(lua_state);
	
	//::testing::InitGoogleTest(&argc, argv);
	//RUN_ALL_TESTS();

	signal(SIGSEGV, SignalHandler);

	try {
		if (!lua_state.dofile("init.lua"))		
			lua_state.debug_response();
	}
	catch (char* e) {
		cout << "Exception thrown! " << e << "\n";
		lua_state.debug_response();
	}

	framework::deinit();
	return 0;
}