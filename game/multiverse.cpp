#include "multiverse.h"
#include "game_window.h"
#include "cosmos.h"
#include "game/types_specification/all_component_includes.h"

multiverse::multiverse() 
	: main_cosmos_timer(60, 5)
{
}

void multiverse::populate_cosmoi() {
	main_cosmos.reserve_storage_for_entities(50000);

	main_cosmos.advance_deterministic_schemata(cosmic_entropy(), [this](fixed_step& step) {
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

	while (steps_to_perform--) {
		auto machine_entropy_for_this_step = main_cosmos_player.obtain_machine_entropy_for_next_step();
		auto cosmic_entropy_for_this_step = main_cosmos_manager.make_cosmic_entropy(machine_entropy_for_this_step, main_cosmos);

		renderer::get_current().clear_logic_lines();

		main_cosmos.delta = main_cosmos_timer.get_fixed_delta();

		main_cosmos.advance_deterministic_schemata(cosmic_entropy_for_this_step,
			[this](fixed_step& step) { main_cosmos_manager.pre_solve(step); },
			[this](fixed_step& step) { main_cosmos_manager.post_solve(step); }
		);

		main_cosmos_timer.increment_total_steps_passed();
	}
}

void multiverse::view(game_window& window) const {
	frame_profiler.fps_counter.new_measurement();

	auto& target = renderer::get_current();

	target.clear_current_fbo();

	target.set_viewport({0, 0, main_cosmos.settings.screen_size.x, main_cosmos.settings.screen_size.y });

	basic_viewing_step main_cosmos_viewing_step(main_cosmos, frame_timer.extract_variable_delta(main_cosmos_timer), target);
	main_cosmos_manager.view_cosmos(main_cosmos_viewing_step);

	frame_profiler.triangles.measure(target.triangles_drawn_total);
	target.triangles_drawn_total = 0;

	window.window.swap_buffers();

	frame_profiler.fps_counter.end_measurement();
}