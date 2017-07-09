#include <thread>
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/assets/assets_manager.h"

#include "game/hardcoded_content/test_scenes/testbed.h"
#include "game/hardcoded_content/test_scenes/minimal_scene.h"

#include "game/transcendental/types_specification/all_component_includes.h"

#include "game/view/viewing_session.h"
#include "game/view/debug_character_selection.h"

#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#include "augs/misc/debug_entropy_player.h"

#include "augs/templates/string_templates.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/log_color.h"
#include "local_setup.h"
#include "game/detail/visible_entities.h"
#include "application/config_lua_table.h"

#include "generated/introspectors.h"

using namespace augs::window::event::keys;

void local_setup::process(
	game_window& window,
	viewing_session& session
) {
	cosmos hypersomnia(3000);
	
	cosmic_entropy total_collected_entropy;
	augs::debug_entropy_player<cosmic_entropy> player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	debug_character_selection characters;

	const auto metas_of_assets = get_assets_manager().generate_logical_metas_of_assets();

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(session.config.default_tickrate);
		
		if (session.config.debug_minimal_test_scene) {
			test_scenes::minimal_scene().populate_world_with_entities(
				hypersomnia,
				metas_of_assets,
				session.get_standard_post_solve()
			);
		}
		else {
			test_scenes::testbed().populate_world_with_entities(
				hypersomnia, 
				metas_of_assets,
				session.get_standard_post_solve()
			);
		}
	}

	characters.acquire_available_characters(hypersomnia);

	hypersomnia.get_entity_by_name(L"player0").set_name(::to_wstring(session.config.nickname));

	const auto player1 = hypersomnia.get_entity_by_name(L"player1");

	if (player1.alive()) {
		player1.set_name(::to_wstring(session.config.debug_second_nickname));
	}

	if (session.config.get_input_recording_mode() != input_recording_type::DISABLED) {
		if (player.try_to_load_or_save_new_session("generated/sessions/", "recorded.inputs")) {
			timer.set_stepping_speed_multiplier(session.config.recording_replay_speed);
		}
	}

	timer.reset_timer();

	const bool debug_control_timing = true;// player.is_replaying();

	while (!should_quit) {
		sync_config_back(session.config, window.window);
		
		const auto screen_size = window.window.get_screen_size();

		augs::renderer::get_current().resize_fbos(screen_size);
		session.set_screen_size(screen_size);

		{
			augs::machine_entropy new_machine_entropy;

			session.local_entropy_profiler.new_measurement();
			new_machine_entropy.local = window.collect_entropy(session.config.enable_cursor_clipping);
			session.local_entropy_profiler.end_measurement();
			
			process_exit_key(new_machine_entropy.local);

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
				hypersomnia[characters.get_selected_character()],
				new_machine_entropy.local
			);
			
			characters.control_character_selection_numeric(new_machine_entropy.local);

			auto translated = session.config.controls.translate(new_machine_entropy.local);

			session.control_and_remove_fetched_intents(translated.intents);
			characters.control_character_selection(translated.intents);

			const auto new_cosmic_entropy = cosmic_entropy(
				hypersomnia[characters.get_selected_character()],
				translated
			);

			total_collected_entropy += new_cosmic_entropy;
		}

		thread_local visible_entities all_visible;

		auto advance_audiovisuals = [&](){
			session.get_visible_entities(all_visible, hypersomnia);

			const augs::delta vdt = 
				timer.get_stepping_speed_multiplier() 
				* session.frame_timer.extract<std::chrono::milliseconds>()
			;

			session.advance_audiovisual_systems(
				hypersomnia, 
				characters.get_selected_character(),
				all_visible,
				vdt
			);
		};

		auto steps = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());

		if (!steps) {
			advance_audiovisuals();
		}

		while (steps--) {
			total_collected_entropy += session.systems_audiovisual.get<gui_element_system>().get_and_clear_pending_events();

			player.advance_player_and_biserialize(total_collected_entropy);

			augs::renderer::get_current().clear_logic_lines();

			hypersomnia.advance_deterministic_schemata(
				{ total_collected_entropy, metas_of_assets },
				[](auto) {},
				session.get_standard_post_solve()
			);

			total_collected_entropy.clear();
			advance_audiovisuals();
		}

		auto& renderer = augs::renderer::get_current();
		renderer.clear_current_fbo();

		session.view(
			renderer, 
			hypersomnia, 
			characters.get_selected_character(), 
			all_visible,
			timer.fraction_of_step_until_next_step(hypersomnia.get_fixed_delta()),
			augs::gui::text::format(L"", augs::gui::text::style(assets::font_id::GUI_FONT))
		);

		window.swap_buffers();
	}
}