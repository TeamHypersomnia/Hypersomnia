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

void hypersomnia_overworld::set_scene_builder(std::unique_ptr<scene_builder> builder) {
	current_scene_builder = std::move(builder);
}

void hypersomnia_overworld::initialize_scene() {
	current_scene_builder->initialize(game_world);
	clear_window_inputs_once = true;
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

		if (clear_window_inputs_once) {
			raw_inputs.clear();
			clear_window_inputs_once = false;
		}

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

		game_window.clear();

		update_frame_timer();
		game_world.draw();
		current_scene_builder->draw(game_world);

		game_window.swap_buffers();

		auto steps_to_do = accumulator.update_and_extract_steps();

		while (steps_to_do--) {
			game_world.perform_logic_step();
			current_scene_builder->perform_logic_step(game_world);
		}
	}
}

void hypersomnia_overworld::configure_scripting() {
	framework::bind_whole_engine(lua);
	bind_whole_hypersomnia(lua);

	game_world.bind_this_to_lua_global(lua, "WORLD");

	signal(SIGSEGV, SignalHandler);
}

