#include "hypersomnia_overworld.h"

#include <signal.h>
#include <iostream>

#include "window_framework/window.h"
#include "script.h"

#include "game_framework/game_framework.h"
#include "game_framework/messages/raw_window_input_message.h"

#include "bindings.h"

#include <luabind/luabind.hpp>

using namespace std;
using namespace augs;

void SignalHandler(int signal) { throw "Access violation!"; }

hypersomnia_overworld::hypersomnia_overworld() 
	: game_world(*this) {
}

void hypersomnia_overworld::initialize() {

}

void hypersomnia_overworld::call_window_script(std::string filename) {
	lua.global_ptr("global_gl_window", &game_window);

	try {
		if (!lua.dofile(filename))
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

	game_window.initial_gl_calls();
}

void hypersomnia_overworld::simulate() {
	bool quit_flag = false;

	while (!quit_flag) {
		auto raw_inputs = game_window.poll_events();

		for (auto& raw_input : raw_inputs) {
			if (raw_input.key_event == window::event::PRESSED &&
				raw_input.key == window::event::keys::ESC) {
				quit_flag = true;
				break;
			}

			messages::raw_window_input_message msg;
			msg.raw_window_input = raw_input;

			game_world.post_message(msg);
		}

		auto steps_to_do = accumulator.update_and_extract_steps();

		while (steps_to_do--) {
			game_world.perform_logic_step();
		}

		game_window.clear();

		game_world.draw();

		game_window.swap_buffers();
	}
}

void hypersomnia_overworld::configure_scripting() {
	framework::bind_whole_engine(lua);
	bind_whole_hypersomnia(lua);

	game_world.bind_this_to_lua_global(lua, "WORLD");

	signal(SIGSEGV, SignalHandler);
}

