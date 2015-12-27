#pragma once
#include "utilities/lua_state_wrapper.h"
#include "game_framework/game_framework.h"

#include "gui\hypersomnia_gui.h"
#include "augmentations.h"

#include "game/utilities.h"
#include "game/bindings.h"

#include <iostream>
#include <signal.h>
#include "script.h"

#include "game/hypersomnia_world.h"

using namespace std;
using namespace augs;

void SignalHandler(int signal) { throw "Access violation!"; }

int main(int argc, char** argv) {
	framework::init();

	augs::lua_state_wrapper lua;

	framework::bind_whole_engine(lua);
	bind_whole_hypersomnia(lua);

	hypersomnia_world new_world;
	new_world.bind_this_to_lua_global(lua, "WORLD");

	framework::run_tests();

	signal(SIGSEGV, SignalHandler);

	try {
		if (!lua.dofile("init.lua"))
			lua.debug_response();
	}
	catch (char* e) {
		cout << "Exception thrown! " << e << "\n";
		lua.debug_response();
	}
	catch (...) {
		cout << "Exception thrown! " << "\n";
		lua.debug_response();
	}

	new_world.simulate();

	framework::deinit();
	return 0;
}