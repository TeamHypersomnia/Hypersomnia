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

#include "game/transcendental/step_and_entropy_unpacker.h"

#include "augs/filesystem/file.h"
#include "local_setup.h"

void local_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_rect());
	const auto& cfg = window.config;

	cosmos hypersomnia(3000);
	hypersomnia.systems_insignificant.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;

	step_and_entropy_unpacker input_unpacker;
	scene_managers::testbed testbed;
	testbed.debug_var = window.config.debug_var;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(augs::fixed_delta(cfg.tickrate));
		testbed.populate_world_with_entities(hypersomnia);
	}

	if (input_unpacker.try_to_load_or_save_new_session("sessions/", "recorded.inputs")) {
		input_unpacker.timer.set_stepping_speed_multiplier(cfg.recording_replay_speed);
	}

	viewing_session session;
	session.camera.configure_size(screen_size);

	testbed.configure_view(session);

	input_unpacker.timer.reset_timer();

	while (!should_quit) {
		augs::machine_entropy new_entropy;

		new_entropy.local = window.collect_entropy();
		session.control(new_entropy);

		process_exit_key(new_entropy.local);

		input_unpacker.control(new_entropy);

		auto steps = input_unpacker.unpack_steps(hypersomnia.get_fixed_delta());

		for (const auto& s : steps) {
			for (const auto& raw_input : s.total_entropy.local) {
				if (raw_input.was_key_pressed()) {
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

			testbed.control(s.total_entropy.local, hypersomnia);

			auto cosmic_entropy_for_this_step = testbed.make_cosmic_entropy(s.total_entropy.local, session.context, hypersomnia);

			renderer::get_current().clear_logic_lines();

			testbed.step_with_callbacks(cosmic_entropy_for_this_step, hypersomnia, session);
		}

		const auto vdt = session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), input_unpacker.timer);

		hypersomnia.integrate_interpolated_transforms(vdt.in_seconds());

		session.view(hypersomnia, testbed.get_controlled_entity(), window, vdt);
	}
}