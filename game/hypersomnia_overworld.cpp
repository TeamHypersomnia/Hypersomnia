#include "game_overworld.h"

#include <signal.h>
#include <iostream>

#include "window_framework/window.h"
#include "script.h"

#include "game_framework/game_framework.h"
#include "game_framework/messages/raw_window_input_message.h"
#include "game_framework/messages/camera_render_request_message.h"

#include "game_framework/systems/render_system.h"

#include "bindings.h"

#include <luabind/luabind.hpp>

using namespace std;
using namespace augs;

void SignalHandler(int signal) { throw "Access violation!"; }

game_overworld::game_overworld() 
	: main_game_world(*this) {
	main_game_world.initialize_entity_component_pools(50000);
}

void game_overworld::set_scene_builder(std::unique_ptr<scene_builder> builder) {
	current_scene_builder = std::move(builder);
}

void game_overworld::initialize_scene() {
	current_scene_builder->initialize(main_game_world);
	clear_window_inputs_once = true;
}

void game_overworld::call_window_script(std::string filename) {
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

#define DETERMINISTIC_RENDERING 0

void game_overworld::simulate() {
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

			main_game_world.post_message(msg);
		}

#if DETERMINISTIC_RENDERING
		auto steps_to_do = accumulator.update_and_extract_steps();

		while (steps_to_do--) {
#endif

		game_window.clear();

		assign_frame_time_to_delta();
		main_game_world.draw();

		consume_camera_render_requests();

		main_game_world.restore_transforms_after_rendering();

		restore_fixed_delta();

#if !DETERMINISTIC_RENDERING
		auto steps_to_do = accumulator.update_and_extract_steps();

		while (steps_to_do--) {
#endif
			renderer::get_current().clear_logic_lines();

			main_game_world.perform_logic_step();
			current_scene_builder->perform_logic_step(main_game_world);
		}
	}
}

void game_overworld::consume_camera_render_requests() {
	auto& requests = main_game_world.get_message_queue<messages::camera_render_request_message>();
	auto& target = renderer::get_current();

	for (auto& r : requests) {
		target.viewport(r.state.viewport);
		current_scene_builder->execute_drawcall_script(r);
		target.draw_debug_info(r.state.visible_world_area, r.state.camera_transform, assets::texture_id::BLANK, main_game_world.get_system<render_system>().targets, view_interpolation_ratio());
	}
	
	current_scene_builder->custom_drawcalls(main_game_world);

	game_window.swap_buffers();
	target.clear_geometry();
}

void game_overworld::configure_scripting() {
	framework::bind_whole_engine(lua);
	bind_whole_hypersomnia(lua);

	main_game_world.bind_this_to_lua_global(lua, "WORLD");

	signal(SIGSEGV, SignalHandler);
}

