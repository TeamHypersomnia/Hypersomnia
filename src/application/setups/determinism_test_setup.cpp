#include <thread>
#include "augs/global_libraries.h"
#include "augs/window_framework/window.h"

#include "game/assets/assets_manager.h"

#include "test_scenes/scenes/testbed.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/data_living_one_step.h"

#include "augs/misc/debug_entropy_player.h"
#include "game/transcendental/logic_step.h"

#include "augs/filesystem/file.h"
#include "determinism_test_setup.h"

#include "augs/misc/templated_readwrite.h"
#include "game/detail/visible_entities.h"

#include "generated/introspectors.h"
#include "game/view/debug_character_selection.h"
#include "application/config_lua_table.h"

void determinism_test_setup::process(
	game_window& window,
	viewing_session& session
) {
	const auto metas_of_assets_unique = get_assets_manager().generate_logical_metas_of_assets();
	const auto& metas_of_assets = *metas_of_assets_unique;

	const vec2i screen_size = vec2i(window.get_screen_size());

	const unsigned cosmoi_count = 1 + session.config.determinism_test_cloned_cosmoi_count;
	std::vector<cosmos> hypersomnias(cosmoi_count, cosmos(3000));

	cosmic_entropy total_collected_entropy;
	augs::debug_entropy_player<cosmic_entropy> player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	std::vector<debug_character_selection> characters(cosmoi_count);

	if (augs::file_exists("save.state")) {
		for (auto& h : hypersomnias) {
			ensure(h.load_from_file("save.state"));
		}
	}
	else {
		for (size_t i = 0; i < cosmoi_count; ++i) {
			hypersomnias[i].set_fixed_delta(session.config.default_tickrate);
			test_scenes::testbed().populate_world_with_entities(
				hypersomnias[i], 
				metas_of_assets,
				session.get_standard_post_solve()
			);

			characters[i].acquire_available_characters(hypersomnias[i]);
		}
	}

	for (auto& h : hypersomnias) {
		ensure(h == hypersomnias[0]);
	}

	if (session.config.get_input_recording_mode() != input_recording_type::DISABLED) {
		if (player.try_to_load_or_save_new_session("generated/sessions/", "recorded.inputs")) {
			timer.set_stepping_speed_multiplier(1.f);
		}
	}

	unsigned currently_viewn_cosmos = 0;
	bool divergence_detected = false;
	unsigned which_divergent = 0;

	timer.reset_timer();

	while (!should_quit) {
		augs::machine_entropy new_machine_entropy;

		new_machine_entropy.local = window.collect_entropy();
		
		if (process_exit(new_machine_entropy.local)) {
			break;
		}

		for (auto& n : new_machine_entropy.local) {
			if (n.was_any_key_pressed()) {
				if (n.key == augs::event::keys::key::F3) {
					++currently_viewn_cosmos;
					currently_viewn_cosmos %= cosmoi_count;
				}
			}
		}

		session.switch_between_gui_and_back(new_machine_entropy.local);

		session.fetch_gui_events(
			hypersomnias[0][characters[0].get_selected_character()],
			new_machine_entropy.local
		);

		auto translated = session.config.controls.translate(new_machine_entropy.local);

		session.fetch_session_intents(translated.intents);

		auto new_cosmic_entropy = cosmic_entropy(
			hypersomnias[0][characters[0].get_selected_character()],
			translated
		);

		new_cosmic_entropy += session.systems_audiovisual.get<gui_element_system>().get_and_clear_pending_events();

		total_collected_entropy += new_cosmic_entropy;

		auto steps = timer.count_logic_steps_to_perform(hypersomnias[0].get_fixed_delta());

		while (steps--) {
			if (divergence_detected) {
				break;
			}

			for (size_t i = 0; i < cosmoi_count; ++i) {
				auto& h = hypersomnias[i];
				
				if (i + 1 < cosmoi_count) {
					hypersomnias[i] = hypersomnias[i + 1];
				}

				characters[i].control_character_selection(translated.intents);

				player.advance_player_and_biserialize(total_collected_entropy);

				augs::renderer::get_current().clear_logic_lines();

				h.advance_deterministic_schemata(
					{ total_collected_entropy, metas_of_assets },
					[](auto) {},
					[this, &session](const const_logic_step step) {
						session.spread_past_infection(step);
					}
				);
			}

			auto& first_cosm = hypersomnias[0].reserved_memory_for_serialization;

			augs::output_stream_reserver first_cosm_reserver;
			augs::write(first_cosm_reserver, hypersomnias[0].significant);
			first_cosm.reserve(first_cosm_reserver.get_write_pos());
			first_cosm.reset_write_pos();
			augs::write(first_cosm, hypersomnias[0].significant);

			for (unsigned i = 1; i < cosmoi_count; ++i) {
				auto& second_cosm = hypersomnias[i].reserved_memory_for_serialization;

				augs::output_stream_reserver second_cosm_reserver;
				augs::write(second_cosm_reserver, hypersomnias[i].significant);
				second_cosm.reserve(second_cosm_reserver.get_write_pos());
				second_cosm.reset_write_pos();
				augs::write(second_cosm, hypersomnias[i].significant);

				if (!(first_cosm == second_cosm)) {
					divergence_detected = true;
					which_divergent = i;
					break;
				}
			}

			total_collected_entropy.clear();
		}

		std::string logged;

		if (divergence_detected) {
			logged += typesafe_sprintf("Divergence detected in cosmos: %x (step: %x)\n", which_divergent, hypersomnias[0].get_total_steps_passed());
		}

		logged += typesafe_sprintf("Currently viewn cosmos: %x (F3 to switch)\n", currently_viewn_cosmos);

		thread_local visible_entities all_visible;
		session.get_visible_entities(all_visible, hypersomnias[currently_viewn_cosmos]);
		
		const augs::delta vdt =
			timer.get_stepping_speed_multiplier()
			* session.frame_timer.extract<std::chrono::milliseconds>()
		;

		session.advance_audiovisual_systems(
			hypersomnias[currently_viewn_cosmos],
			characters[currently_viewn_cosmos].get_selected_character(),
			all_visible,
			vdt
		);

		auto& renderer = augs::renderer::get_current();
		renderer.clear_current_fbo();

		session.view(
			renderer, 
			hypersomnias[currently_viewn_cosmos], 
			characters[currently_viewn_cosmos].get_selected_character(),
			all_visible,
			timer.fraction_of_step_until_next_step(hypersomnias[currently_viewn_cosmos].get_fixed_delta())
		);
	
		window.swap_buffers();
	}
}