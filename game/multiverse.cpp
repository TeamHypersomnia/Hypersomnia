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
#include "game_window.h"
#include "cosmos.h"
#include "types_specification/all_component_includes.h"

multiverse::multiverse() 
	: main_cosmos_timer(60, 5)
{
	main_cosmos = cosmos();
	main_cosmos.reserve_storage_for_entities(50000);

	main_cosmos.advance_deterministic_schemata(augs::machine_entropy(), [this](fixed_step& step) {
		main_cosmos_manager.populate_world_with_entities(step);
	});
}

void multiverse::control(augs::machine_entropy entropy) {
	main_cosmos_player.buffer_entropy_for_next_step(entropy);

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

	main_cosmos_timer.set_stepping_speed_multiplier(stepping_speed);
}

void multiverse::simulate() {
	auto steps_to_perform = main_cosmos_timer.count_logic_steps_to_perform();

	if (steps_to_perform > 0) {
		auto total_entropy_for_this_step = main_cosmos_player.obtain_total_entropy_for_next_step();

		while (steps_to_perform--) {
			renderer::get_current().clear_logic_lines();
			
			main_cosmos.delta = main_cosmos_timer.get_fixed_delta();

			main_cosmos.advance_deterministic_schemata(total_entropy_for_this_step, 
				[this](fixed_step& step) { main_cosmos_manager.pre_solve(step); }, 
				[this](fixed_step& step) { main_cosmos_manager.post_solve(step); }
			);

			main_cosmos_timer.increment_total_steps_passed();
		}
	}
}

void multiverse::view(game_window& window) const {
	main_cosmos.profiler.fps_counter.new_measurement();

	auto& target = renderer::get_current();

	target.clear_current_fbo();

	main_cosmos.call_rendering_schemata(frame_timer.extract_variable_delta(main_cosmos_timer),
		cosmos::variable_callback(),
		[this, &target](variable_step& step) {
			auto& requests = step.messages.get_queue<messages::camera_render_request_message>();

			for (auto& r : requests) {
				target.set_viewport(r.state.viewport);
				main_cosmos_manager.execute_drawcalls_for_camera(r);
			}

			main_cosmos_manager.drawcalls_after_all_cameras(step);
	
	});

	main_cosmos.profiler.triangles.measure(target.triangles_drawn_total);
	target.triangles_drawn_total = 0;

	window.window.swap_buffers();

	main_cosmos.profiler.fps_counter.end_measurement();
}
