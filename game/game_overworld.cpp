#include "game_overworld.h"

#include <signal.h>

#include "window_framework/window.h"
#include "script.h"

#include "game/bind_game_and_augs.h"
#include "game/messages/raw_window_input_message.h"
#include "game/messages/camera_render_request_message.h"

#include "game/systems/input_system.h"
#include "game/systems/render_system.h"
#include "game/systems/gui_system.h"
#include "game/resources/manager.h"

#include <luabind/luabind.hpp>

#include "log.h"

using namespace std;
using namespace augs;

void SignalHandler(int signal) { 
	ClipCursor(NULL);
	throw "Access violation!"; 
}

game_overworld::game_overworld() 
	: main_game_world(*this) {
	main_game_world.initialize_entity_and_component_pools(50000);
}

void game_overworld::configure_scripting() {
	bind_game_and_augs(lua);
	main_game_world.bind_this_to_lua_global(lua, "WORLD");

	signal(SIGSEGV, SignalHandler);
}

void game_overworld::call_window_script(std::string filename) {
	lua.global_ptr("global_gl_window", &game_window);

	try {
		if (!lua.dofile(filename))
			lua.debug_response();
	}
	catch (char* e) {
		LOG("Exception thrown! %x", e);
		lua.debug_response();
	}
	catch (...) {
		LOG("Exception thrown!");
		lua.debug_response();
	}

	game_window.initial_gl_calls();
}

void game_overworld::set_scene_builder(std::unique_ptr<scene_builder> builder) {
	current_scene_builder = std::move(builder);
}

void game_overworld::build_scene() {
	resource_manager.destroy_everything();
	current_scene_builder->load_resources();
	
	main_game_world.delete_all_entities();
	current_scene_builder->populate_world_with_entities(main_game_world);
	
	clear_window_inputs_once = true;
}

#define RENDERING_STEPS_DETERMINISTICALLY_LIKE_LOGIC 0

float stepping_speed = 1.f;
void game_overworld::main_game_loop() {
	bool quit_flag = false;

	while (!quit_flag) {
		main_game_world.get_message_queue<messages::raw_window_input_message>().clear();

		auto raw_window_inputs = game_window.poll_events();

		if (clear_window_inputs_once) {
			raw_window_inputs.clear();
			clear_window_inputs_once = false;
		}

		for (auto& raw_input : raw_window_inputs) {
			if (raw_input.key_event == window::event::PRESSED) {
				if (raw_input.key == window::event::keys::ESC) {
					quit_flag = true;
					break;
				}
				if (raw_input.key == window::event::keys::_1) {
					configure_stepping(60, 500000);
				}
				if (raw_input.key == window::event::keys::_2) {
					configure_stepping(128, 500000);
				}
				if (raw_input.key == window::event::keys::_3) {
					configure_stepping(400, 500000);
				}
				if (raw_input.key == window::event::keys::_4) {
					stepping_speed = 0.1f;
				}
				if (raw_input.key == window::event::keys::_5) {
					stepping_speed = 1.f;
				}
				if (raw_input.key == window::event::keys::F4) {
					LOG_COLOR(console_color::YELLOW, "Separator");
				}
			}

			messages::raw_window_input_message msg;
			msg.raw_window_input = raw_input;

			if(!main_game_world.get_system<input_system>().is_replaying())
				main_game_world.post_message(msg);
		}

		if (!main_game_world.get_system<input_system>().is_replaying())
			delta_timer.set_stepping_speed_multiplier(stepping_speed);

#if RENDERING_STEPS_DETERMINISTICALLY_LIKE_LOGIC
		auto steps_to_perform = delta_timer.count_logic_steps_to_perform();

		while (steps_to_perform--) {
#endif

		game_window.clear();

		assign_frame_time_to_delta_for_drawing_time_systems();
		main_game_world.call_drawing_time_systems();

		consume_camera_render_requests();

		main_game_world.restore_transforms_after_drawing();

		restore_fixed_delta();

#if !RENDERING_STEPS_DETERMINISTICALLY_LIKE_LOGIC
		auto steps_to_perform = delta_timer.count_logic_steps_to_perform();

		while (steps_to_perform--) {
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
		target.set_viewport(r.state.viewport);
		current_scene_builder->execute_drawcalls_for_camera(r);
	}

	current_scene_builder->drawcalls_after_all_cameras(main_game_world);

	game_window.swap_buffers();
}
