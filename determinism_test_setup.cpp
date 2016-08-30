#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/simulation_exchange.h"
#include "game/transcendental/cosmos.h"

#include "game/transcendental/step_and_entropy_unpacker.h"

#include "augs/filesystem/file.h"
#include "setups.h"

#include "augs/misc/templated_readwrite.h"

void determinism_test_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_rect());

	const unsigned cosmoi_count = 1 + static_cast<unsigned>(window.get_config_number("determinism_test_cloned_cosmoi_count"));
	std::vector<cosmos> hypersomnias(cosmoi_count, cosmos(3000));

	step_and_entropy_unpacker input_unpacker;
	std::vector<scene_managers::testbed> testbeds(cosmoi_count);

	const auto config_tickrate = static_cast<unsigned>(window.get_config_number("tickrate"));

	if (augs::file_exists("save.state")) {
		for (auto& h : hypersomnias) {
			ensure(h.load_from_file("save.state"));
		}
	}
	else {
		for (size_t i = 0; i < cosmoi_count; ++i) {
			hypersomnias[i].set_fixed_delta(augs::fixed_delta(config_tickrate));
			testbeds[i].populate_world_with_entities(hypersomnias[i]);
		}
	}

	for (auto& h : hypersomnias) {
		ensure(h == hypersomnias[0]);
	}

	input_unpacker.try_to_load_or_save_new_session("sessions/", "recorded.inputs");

	viewing_session session;
	session.camera.configure_size(screen_size);

	for (size_t i = 0; i < cosmoi_count; ++i) {
		testbeds[i].configure_view(session);
	}

	unsigned currently_viewn_cosmos = 0;
	bool divergence_detected = false;
	unsigned which_divergent = 0;

	input_unpacker.timer.reset_timer();

	while (!should_quit) {
		augs::machine_entropy new_entropy;

		new_entropy.local = window.collect_entropy();
		
		if (process_exit_key(new_entropy.local))
			break;

		for (auto& n : new_entropy.local) {
			if (n.key_event == augs::window::event::key_changed::PRESSED) {
				if (n.key == augs::window::event::keys::F3) {
					++currently_viewn_cosmos;
					currently_viewn_cosmos %= cosmoi_count;
				}
			}
		}

		input_unpacker.control(new_entropy);

		auto steps = input_unpacker.unpack_steps(hypersomnias[0].get_fixed_delta());

		for (const auto& s : steps) {
			if (divergence_detected)
				break;

			for (size_t i = 0; i < cosmoi_count; ++i) {
				auto& h = hypersomnias[i];
				testbeds[i].control(s.total_entropy.local, h);

				auto cosmic_entropy_for_this_step = testbeds[i].make_cosmic_entropy(s.total_entropy.local, session.input, h);

				testbeds[i].step_with_callbacks(cosmic_entropy_for_this_step, h);

				renderer::get_current().clear_logic_lines();
			}

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

		testbeds[currently_viewn_cosmos].view(hypersomnias[currently_viewn_cosmos], window, session, session.frame_timer.extract_variable_delta(hypersomnias[currently_viewn_cosmos].get_fixed_delta(), input_unpacker.timer),
			logged);
	}
}