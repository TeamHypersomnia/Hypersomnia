#include "game/transcendental/entity_relations.h"
#include "game/game_window.h"
#include "multiverse.h"
#include "cosmos.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/gui/text/printer.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"
#include "game/transcendental/step.h"
#include "game/transcendental/viewing_session.h"

multiverse::multiverse() 
	: main_cosmos_timer(60, 5), stashed_timer(main_cosmos_timer)
{
}

bool multiverse::try_to_load_or_save_new_session() {
	if (!main_cosmos_player.try_to_load_and_replay_recording(recording_filename)) {
		main_cosmos_player.record_and_save_this_session(sessions_folder, recording_filename);

		return false;
	}

	return true;
}

bool multiverse::try_to_load_save() {
	if (augs::file_exists(save_filename)) {
		load_cosmos_from_file(save_filename);
		return true;
	}

	return false;
}

void multiverse::populate_cosmoi() {
	main_cosmos.reserve_storage_for_entities(50000);
	
	main_cosmos.significant.delta = main_cosmos_timer.get_fixed_delta();

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
			if (raw_input.key == window::event::keys::DASH) {
				show_profile_details = !show_profile_details;
			}
			if (raw_input.key == window::event::keys::_2) {
				main_cosmos_timer = augs::fixed_delta_timer(128, 500000);
			}
			if (raw_input.key == window::event::keys::_3) {
				main_cosmos_timer = augs::fixed_delta_timer(144, 500000);
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

		for (auto& raw_input : machine_entropy_for_this_step.local) {
			if (raw_input.key_event == window::event::PRESSED) {
				if (raw_input.key == window::event::keys::F7) {
					auto target_folder = saves_folder + augs::get_timestamp();
					augs::create_directories(target_folder);

					save_cosmos_to_file(target_folder + "/" + save_filename);
				}
				if (raw_input.key == window::event::keys::F8) {
					duplication.new_measurement();
					stashed_cosmos = main_cosmos;
					stashed_timer = main_cosmos_timer;
					duplication.end_measurement();
				}
				if (raw_input.key == window::event::keys::F9) {
					duplication.new_measurement();
					main_cosmos = stashed_cosmos;
					main_cosmos_timer = stashed_timer;
					duplication.end_measurement();
					LOG_COLOR(console_color::YELLOW, "Separator");
				}
				if (raw_input.key == window::event::keys::F10) {
					main_cosmos.significant.settings.enable_interpolation = !main_cosmos.significant.settings.enable_interpolation;
				}
			}
		}

		auto cosmic_entropy_for_this_step = main_cosmos_manager.make_cosmic_entropy(machine_entropy_for_this_step, main_cosmos);

		renderer::get_current().clear_logic_lines();

		main_cosmos.significant.delta = main_cosmos_timer.get_fixed_delta();

		main_cosmos.advance_deterministic_schemata(cosmic_entropy_for_this_step,
			[this](fixed_step& step) { main_cosmos_manager.pre_solve(step); },
			[this](fixed_step& step) { main_cosmos_manager.post_solve(step); }
		);

		main_cosmos_timer.increment_total_steps_passed();
	}
}

void multiverse::view(game_window& window, viewing_session& session) const {
	session.fps_profiler.new_measurement();

	auto& target = renderer::get_current();

	target.clear_current_fbo();

	target.set_viewport({0, 0, static_cast<int>(session.camera.visible_world_area.x), static_cast<int>(session.camera.visible_world_area.y) });

	basic_viewing_step main_cosmos_viewing_step(main_cosmos, session.frame_timer.extract_variable_delta(main_cosmos_timer), target);
	main_cosmos_manager.view_cosmos(main_cosmos_viewing_step, session.camera);
	
	print_summary(target, session);

	session.triangles.measure(static_cast<double>(target.triangles_drawn_total));
	target.triangles_drawn_total = 0;

	window.window.swap_buffers();

	session.fps_profiler.end_measurement();
}


std::wstring multiverse::summary(bool detailed, const viewing_session& session) const {
	std::wstring result; 
	result += typesafe_sprintf(L"Entities: %x\n", main_cosmos.entities_count());
	result += session.fps_profiler.summary();
	result += session.triangles.summary();
	
	if (detailed) {
		result += main_cosmos.profiler.sorted_summary();
	}

	result += total_save.summary();
	result += size_calculation_pass.summary();
	result += memory_allocation_pass.summary();
	result += serialization_pass.summary();
	result += writing_savefile.summary();

	result += total_load.summary();
	result += reading_savefile.summary();
	result += deserialization_pass.summary();

	result += duplication.summary();

	return result;
}

void multiverse::print_summary(augs::renderer& target, const viewing_session& session) const {
	auto& cosmos = main_cosmos;
	using namespace augs::gui::text;

	auto controlled = cosmos[main_cosmos_manager.get_controlled_entity()];

	auto coords = controlled.get<components::transform>().pos;
	auto vel = controlled.get<components::physics>().velocity();

	quick_print_format(target.triangles, typesafe_sprintf(L"X: %f2\nY: %f2\nVelX: %x\nVelY: %x\n", coords.x, coords.y, vel.x, vel.y)
		+ summary(show_profile_details, session), style(assets::GUI_FONT, rgba(255, 255, 255, 150)), vec2i(0, 0), 0);

	target.call_triangles();
	target.clear_triangles();
}