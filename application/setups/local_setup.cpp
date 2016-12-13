#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_messages_includes.h"

#include "game/transcendental/step_and_entropy_unpacker.h"

#include "augs/filesystem/file.h"
#include "local_setup.h"

void local_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_rect());
	const auto& cfg = window.config;

	cosmos hypersomnia(3000);

	step_and_entropy_unpacker input_unpacker;
	scene_managers::testbed testbed;
	testbed.debug_var = window.config.debug_var;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(augs::fixed_delta(cfg.tickrate));
		testbed.populate_world_with_entities(hypersomnia, screen_size);
	}

	if (input_unpacker.try_to_load_or_save_new_session(window.get_input_recording_mode(), "sessions/", "recorded.inputs")) {
		input_unpacker.timer.set_stepping_speed_multiplier(cfg.recording_replay_speed);
	}

	viewing_session session;
	session.reserve_caches_for_entities(3000);
	session.camera.configure_size(screen_size);
	session.systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;

	cosmic_movie_director dir;
	dir.load_recordings_from_directory("director/menu");

	testbed.configure_view(session);

	input_unpacker.timer.reset_timer();

	while (!should_quit) {
		augs::machine_entropy new_entropy;

		session.local_entropy_profiler.new_measurement();
		new_entropy.local = window.collect_entropy();
		session.local_entropy_profiler.end_measurement();
		session.control(new_entropy);

		for (const auto& raw_input : new_entropy.local) {
			if (raw_input.was_any_key_pressed()) {
				if (raw_input.key == augs::window::event::keys::key::_1) {
					hypersomnia.set_fixed_delta(cfg.tickrate);
				}
				if (raw_input.key == augs::window::event::keys::key::_2) {
					hypersomnia.set_fixed_delta(128);
				}
				if (raw_input.key == augs::window::event::keys::key::_3) {
					hypersomnia.set_fixed_delta(144);
				}
				if (raw_input.key == augs::window::event::keys::key::_4) {
					input_unpacker.timer.set_stepping_speed_multiplier(0.1f);
				}
				if (raw_input.key == augs::window::event::keys::key::_5) {
					input_unpacker.timer.set_stepping_speed_multiplier(1.f);
				}
				if (raw_input.key == augs::window::event::keys::key::_6) {
					input_unpacker.timer.set_stepping_speed_multiplier(6.f);
				}
				if (raw_input.key == augs::window::event::keys::key::F2) {
					LOG_COLOR(console_color::YELLOW, "Separator");
				}
			}
		}

		process_exit_key(new_entropy.local);

		input_unpacker.control(new_entropy);

		auto steps = input_unpacker.unpack_steps(hypersomnia.get_fixed_delta());

		for (const auto& s : steps) {
			testbed.control(s.total_entropy.local, hypersomnia);

			auto cosmic_entropy_for_this_step = testbed.make_cosmic_entropy(s.total_entropy.local, session.context, hypersomnia);

			renderer::get_current().clear_logic_lines();

			hypersomnia.advance_deterministic_schemata(cosmic_entropy_for_this_step, [](auto){},
				[this, &session](const const_logic_step& step){
					session.acquire_game_events_for_hud(step);
				}
			);

			session.resample_state_for_audiovisuals(hypersomnia);
		}

		const auto vdt = session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), input_unpacker.timer);

		session.advance_audiovisual_systems(hypersomnia, testbed.get_controlled_entity(), vdt);

		session.view(hypersomnia, testbed.get_controlled_entity(), window, vdt);
	}
}