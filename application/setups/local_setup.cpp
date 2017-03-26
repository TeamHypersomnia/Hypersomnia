#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_builders/testbed.h"
#include "game/scene_builders/one_entity.h"
#include "game/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#include "augs/misc/debug_entropy_player.h"

#include "augs/templates/string_templates.h"
#include "augs/filesystem/file.h"
#include "local_setup.h"
#include "augs/tweaker.h"
#include "game/detail/visible_entities.h"
#include "application/config_lua_table.h"

using namespace augs::window::event::keys;

void local_setup::process(
	const config_lua_table& cfg, 
	game_window& window
) {
	const vec2i screen_size = vec2i(window.get_screen_size());

	cosmos hypersomnia(3000);

	viewing_session session;

	session.reserve_caches_for_entities(3000);
	session.set_screen_size(screen_size);
	session.systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;
	session.set_master_gain(cfg.sound_effects_volume);

	session.configure_input();

	const auto standard_post_solve = [&session](const const_logic_step step) {
		session.standard_audiovisual_post_solve(step);
	};

	cosmic_entropy total_collected_entropy;
	augs::debug_entropy_player<cosmic_entropy> player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	scene_builders::testbed testbed;
	testbed.debug_var = cfg.debug_var;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(cfg.default_tickrate);
		
		testbed.populate_world_with_entities(
			hypersomnia, 
			standard_post_solve
		);
	}

	hypersomnia[testbed.characters[0]].get<components::name>().nickname = ::to_wstring(cfg.nickname);

	if (testbed.characters.size() > 1) {
		hypersomnia[testbed.characters[1]].get<components::name>().nickname = ::to_wstring(cfg.debug_second_nickname);
	}

	if (cfg.get_input_recording_mode() != input_recording_type::DISABLED) {
		if (player.try_to_load_or_save_new_session("generated/sessions/", "recorded.inputs")) {
			timer.set_stepping_speed_multiplier(cfg.recording_replay_speed);
		}
	}

	timer.reset_timer();

	const bool debug_control_timing = true;// player.is_replaying();

	while (!should_quit) {
		{
			augs::machine_entropy new_machine_entropy;

			session.local_entropy_profiler.new_measurement();
			new_machine_entropy.local = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
			session.local_entropy_profiler.end_measurement();
			
			process_exit_key(new_machine_entropy.local);
			control_tweaker(new_machine_entropy.local);

			if (debug_control_timing) {
				for (const auto& raw_input : new_machine_entropy.local) {
					if (raw_input.was_any_key_pressed()) {
						if (raw_input.key == key::_4) {
							timer.set_stepping_speed_multiplier(0.1f);
						}
						if (raw_input.key == key::_5) {
							timer.set_stepping_speed_multiplier(1.f);
						}
						if (raw_input.key == key::_6) {
							timer.set_stepping_speed_multiplier(6.f);
						}
					}
				}
			}

			for (const auto& raw_input : new_machine_entropy.local) {
				if (raw_input.was_any_key_pressed()) {
					if (raw_input.key == key::F2) {
						LOG_COLOR(console_color::YELLOW, "Separator");
					}
				}
			}

			session.switch_between_gui_and_back(new_machine_entropy.local);
			
			session.control_gui_and_remove_fetched_events(
				hypersomnia[testbed.get_selected_character()],
				new_machine_entropy.local
			);

			auto new_intents = session.context.to_key_and_mouse_intents(new_machine_entropy.local);

			session.control_and_remove_fetched_intents(new_intents);
			testbed.control_character_selection(new_intents);

			auto new_cosmic_entropy = cosmic_entropy(
				hypersomnia[testbed.get_selected_character()],
				new_intents
			);

			total_collected_entropy += new_cosmic_entropy;
		}

		auto steps = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());

		while (steps--) {
			total_collected_entropy += session.systems_audiovisual.get<gui_element_system>().get_and_clear_pending_events();

			player.advance_player_and_biserialize(total_collected_entropy);

			augs::renderer::get_current().clear_logic_lines();

			hypersomnia.advance_deterministic_schemata(
				total_collected_entropy,
				[](const auto) {},
				standard_post_solve
			);

			total_collected_entropy = cosmic_entropy();
		}

		static thread_local visible_entities all_visible;
		session.get_visible_entities(all_visible, hypersomnia);

		const auto vdt = session.frame_timer.extract_variable_delta(
			hypersomnia.get_fixed_delta(), 
			timer
		);

		session.advance_audiovisual_systems(
			hypersomnia, 
			testbed.get_selected_character(),
			all_visible,
			vdt
		);

		auto& renderer = augs::renderer::get_current();
		renderer.clear_current_fbo();

		session.view(
			cfg,
			renderer, 
			hypersomnia, 
			testbed.get_selected_character(), 
			all_visible,
			timer.fraction_of_step_until_next_step(hypersomnia.get_fixed_delta()),
			augs::gui::text::format(write_tweaker_report().c_str(), augs::gui::text::style(assets::font_id::GUI_FONT))
		);

		window.swap_buffers();
	}
}