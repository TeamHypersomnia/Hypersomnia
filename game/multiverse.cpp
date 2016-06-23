#include "multiverse.h"

#include <signal.h>

#include "window_framework/platform_utils.h"
#include "scripting/script.h"

#include "game/bindings/bind_game_and_augs.h"
#include "game/messages/camera_render_request_message.h"

#include "game/systems/input_system.h"
#include "game/systems/render_system.h"
#include "game/stateful_systems/gui_system.h"
#include "game/resources/manager.h"

#include "game/scene_managers/resource_setups/all.h"

#include <luabind/luabind.hpp>

#include "log.h"

multiverse::multiverse() 
	: main_cosmos_timer(60, 5)
{
	main_cosmos = cosmos();
	main_cosmos.reserve_storage_for_entities(50000);

	step_state initializatory_step;

	main_cosmos_manager.populate_world_with_entities(main_cosmos, initializatory_step);
	main_cosmos.advance_deterministic_schemata(augs::machine_entropy(), initializatory_step);
}

void multiverse::control(augs::machine_entropy entropy) {
	for (auto& raw_input : entropy.local) {
		if (raw_input.key_event == window::event::PRESSED) {
			if (raw_input.key == window::event::keys::_1) {
				main_cosmos_timer = augs::fixed_delta_timer(60, 500000);
			}
			if (raw_input.key == window::event::keys::_2) {
				main_cosmos_timer = augs::fixed_delta_timer(128, 500000);
			}
			if (raw_input.key == window::event::keys::_3) {
				main_cosmos_timer = augs::fixed_delta_timer(400, 500000);
			}
			if (raw_input.key == window::event::keys::_4) {
				stepping_speed = 0.1f;
			}
			if (raw_input.key == window::event::keys::_5) {
				stepping_speed = 1.f;
			}
			if (raw_input.key == window::event::keys::_6) {
				stepping_speed = 6.f;
			}
			if (raw_input.key == window::event::keys::F4) {
				LOG_COLOR(console_color::YELLOW, "Separator");
			}
		}
	}
}

void multiverse::view(game_window& window) const {
	main_cosmos.profiler.fps_counter.new_measurement();

	step_state step;
	main_cosmos.call_rendering_schemata(frame_timer.extract_variable_delta(main_cosmos_timer), step);
	main_cosmos.profiler.fps_counter.end_measurement();
}

void multiverse::simulate() {
	auto steps_to_perform = main_cosmos_timer.count_logic_steps_to_perform();

	if (steps_to_perform > 0) {

		auto total_entropy_for_this_step = main_cosmos_player.acquire_machine_entropy_for_this_step();

		while (steps_to_perform--) {
			renderer::get_current().clear_logic_lines();
			
			main_cosmos.delta = main_cosmos_timer.get_fixed_delta();

			step_state step;
			main_cosmos_manager.pre_solve(main_cosmos, step);

			main_cosmos.advance_deterministic_schemata(total_entropy_for_this_step, step);
			main_cosmos_manager.post_solve(main_cosmos, step);

			main_cosmos_timer.increment_total_steps_passed();
		}
	}



	bool quit_flag = false;

	while (!quit_flag) {

		main_step.messages.get_queue<messages::raw_window_input_message>().clear();

		auto raw_window_inputs = game_window.poll_events();

		if (clear_window_inputs_once) {
			raw_window_inputs.clear();
			clear_window_inputs_once = false;
		}


		delta_timer.set_stepping_speed_multiplier(stepping_speed);

		assign_frame_time_to_delta_for_drawing_time_systems();

		enable_drawing_time_random_generator();
		main_cosmos.call_drawing_time_systems();

		consume_camera_render_requests();

		main_cosmos.restore_transforms_after_drawing();

		restore_fixed_delta();
		enable_deterministic_random_generator();

	}
}

void multiverse::consume_camera_render_requests() {
	auto& requests = main_step.messages.get_queue<messages::camera_render_request_message>();
	auto& target = renderer::get_current();

	target.clear_current_fbo();

	for (auto& r : requests) {
		target.set_viewport(r.state.viewport);
		current_scene_manager->execute_drawcalls_for_camera(r);
	}

	current_scene_manager->drawcalls_after_all_cameras(main_cosmos);
	
	main_cosmos.triangles.measure(target.triangles_drawn_total);
	target.triangles_drawn_total = 0;

	game_window.swap_buffers();
}
