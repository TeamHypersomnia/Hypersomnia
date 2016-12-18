#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/data_living_one_step.h"

#include "augs/misc/machine_entropy_buffer_and_player.h"
#include "game/transcendental/step.h"

#include "augs/filesystem/file.h"
#include "determinism_test_setup.h"

#include "augs/misc/templated_readwrite.h"

void determinism_test_setup::process(game_window& window) {
	const auto& cfg = window.config;
	const vec2i screen_size = vec2i(window.get_screen_rect());

	const unsigned cosmoi_count = 1 + cfg.determinism_test_cloned_cosmoi_count;
	std::vector<cosmos> hypersomnias(cosmoi_count, cosmos(3000));

	augs::machine_entropy_buffer_and_player player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	std::vector<scene_managers::testbed> testbeds(cosmoi_count);

	if (augs::file_exists("save.state")) {
		for (auto& h : hypersomnias) {
			ensure(h.load_from_file("save.state"));
		}
	}
	else {
		for (size_t i = 0; i < cosmoi_count; ++i) {
			hypersomnias[i].set_fixed_delta(augs::fixed_delta(cfg.tickrate));
			testbeds[i].populate_world_with_entities(hypersomnias[i], vec2i(1920, 1080));
		}
	}

	for (auto& h : hypersomnias) {
		ensure(h == hypersomnias[0]);
	}

	if (window.get_input_recording_mode() != input_recording_mode::DISABLED) {
		if (player.try_to_load_or_save_new_session("sessions/", "recorded.inputs")) {
			timer.set_stepping_speed_multiplier(cfg.recording_replay_speed);
		}
	}

	viewing_session session;
	session.reserve_caches_for_entities(3000);
	session.camera.configure_size(screen_size);
	session.set_interpolation_enabled(false);

	for (size_t i = 0; i < cosmoi_count; ++i) {
		testbeds[i].configure_view(session);
	}

	unsigned currently_viewn_cosmos = 0;
	bool divergence_detected = false;
	unsigned which_divergent = 0;

	timer.reset_timer();

	while (!should_quit) {
		augs::machine_entropy new_entropy;

		new_entropy.local = window.collect_entropy();
		
		if (process_exit_key(new_entropy.local))
			break;

		for (auto& n : new_entropy.local) {
			if (n.was_any_key_pressed()) {
				if (n.key == augs::window::event::keys::key::F3) {
					++currently_viewn_cosmos;
					currently_viewn_cosmos %= cosmoi_count;
				}
			}
		}

		player.accumulate_entropy_for_next_step(new_entropy);

		auto steps = timer.count_logic_steps_to_perform(hypersomnias[0].get_fixed_delta());

		while (steps--) {
			if (divergence_detected) {
				break;
			}

			const auto total_entropy = player.obtain_machine_entropy_for_next_step();

			for (size_t i = 0; i < cosmoi_count; ++i) {
				auto& h = hypersomnias[i];
				if (i + 1 < cosmoi_count)
					hypersomnias[i] = hypersomnias[i + 1];

				testbeds[i].control_character_selection(total_entropy.local);

				auto cosmic_entropy_for_this_step = cosmic_entropy(h[testbeds[i].get_selected_character()], total_entropy.local, session.context);

				renderer::get_current().clear_logic_lines();

				h.advance_deterministic_schemata(cosmic_entropy_for_this_step, [](auto) {},
					[this, &session](const const_logic_step& step) {
						session.spread_past_infection(step);
					}
				);
			}

			session.resample_state_for_audiovisuals(hypersomnias[0]);

			auto& first_cosm = hypersomnias[0].reserved_memory_for_serialization;

			augs::output_stream_reserver first_cosm_reserver;
			augs::write_object(first_cosm_reserver, hypersomnias[0].significant);
			first_cosm.reserve(first_cosm_reserver.get_write_pos());
			first_cosm.reset_write_pos();
			augs::write_object(first_cosm, hypersomnias[0].significant);

			for (unsigned i = 1; i < cosmoi_count; ++i) {
				auto& second_cosm = hypersomnias[i].reserved_memory_for_serialization;

				augs::output_stream_reserver second_cosm_reserver;
				augs::write_object(second_cosm_reserver, hypersomnias[i].significant);
				second_cosm.reserve(second_cosm_reserver.get_write_pos());
				second_cosm.reset_write_pos();
				augs::write_object(second_cosm, hypersomnias[i].significant);

				if (!(first_cosm == second_cosm)) {
					divergence_detected = true;
					which_divergent = i;
					break;
				}
			}
		}

		std::string logged;

		if (divergence_detected) {
			logged += typesafe_sprintf("Divergence detected in cosmos: %x (step: %x)\n", which_divergent, hypersomnias[0].get_total_steps_passed());
		}

		logged += typesafe_sprintf("Currently viewn cosmos: %x (F3 to switch)\n", currently_viewn_cosmos);

		session.view(hypersomnias[currently_viewn_cosmos], testbeds[currently_viewn_cosmos].get_selected_character(), window, session.frame_timer.extract_variable_delta(hypersomnias[currently_viewn_cosmos].get_fixed_delta(), timer));
	}
}