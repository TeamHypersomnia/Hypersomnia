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

#include "augs/misc/machine_entropy_buffer_and_player.h"

#include "augs/filesystem/file.h"
#include "director_setup.h"

void director_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_rect());
	const auto& cfg = window.config;

	cosmos hypersomnia(3000);

	augs::window::event::state events;
	augs::machine_entropy_buffer_and_player machine_player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	scene_managers::testbed testbed;
	testbed.debug_var = window.config.debug_var;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(augs::fixed_delta(cfg.tickrate));
		testbed.populate_world_with_entities(hypersomnia, screen_size);
	}

	viewing_session session;
	session.reserve_caches_for_entities(3000);
	session.camera.configure_size(screen_size);
	session.systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;
	
	testbed.configure_view(session);

	cosmic_movie_director director;
	director.load_recording_from_file(cfg.director_scenario_filename);

	enum class director_state {
		PLAYING,
		RECORDING
	} current_director_state = director_state::PLAYING;

	timer.reset_timer();

	int advance_steps_forward = 0;

	float requested_playing_speed = 0.f;

	while (!should_quit) {
		{
			augs::machine_entropy new_entropy;

			session.local_entropy_profiler.new_measurement();
			new_entropy.local = window.collect_entropy();
			session.local_entropy_profiler.end_measurement();
			
			session.control(new_entropy);

			process_exit_key(new_entropy.local);

			machine_player.accumulate_entropy_for_next_step(new_entropy);

			for (const auto& raw_input : new_entropy.local) {
				events.apply(raw_input);

				if (raw_input.was_any_key_pressed()) {
					if (raw_input.key == augs::window::event::keys::key::F2) {
						current_director_state = director_state::PLAYING;
					}
					if (raw_input.key == augs::window::event::keys::key::F3) {
						current_director_state = director_state::RECORDING;
					}

					if (raw_input.key == augs::window::event::keys::key::_1) {
					
					}
					if (raw_input.key == augs::window::event::keys::key::_2) {
						advance_steps_forward = 1;

						if (events.is_set(augs::window::event::keys::key::LCTRL)) {
							advance_steps_forward *= 10;
						}
					}
					if (raw_input.key == augs::window::event::keys::key::_3) {
						requested_playing_speed = 0.f;
					}
					if (raw_input.key == augs::window::event::keys::key::_4) {
						requested_playing_speed = 0.1f;
					}
					if (raw_input.key == augs::window::event::keys::key::_5) {
						requested_playing_speed = 1.f;
					}
					if (raw_input.key == augs::window::event::keys::key::_6) {
						requested_playing_speed = 6.f;
					}

					if (raw_input.key == augs::window::event::keys::key::F2) {
						LOG_COLOR(console_color::YELLOW, "Separator");
					}
				}
			}

			testbed.control_character_selection(new_entropy.local);

			timer.set_stepping_speed_multiplier(requested_playing_speed);
		}

		auto steps = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());
		
		steps += advance_steps_forward;
		advance_steps_forward = 0;

		while (steps--) {
			const auto total_entropy = machine_player.obtain_machine_entropy_for_next_step();
			cosmic_entropy cosmic_entropy_for_this_advancement;

			if (current_director_state == director_state::PLAYING) {
				guid_mapped_entropy replayed_entropy;
				director.player.replay_next_step(replayed_entropy);

				cosmic_entropy_for_this_advancement = cosmic_entropy(replayed_entropy, hypersomnia);
			}

			renderer::get_current().clear_logic_lines();

			hypersomnia.advance_deterministic_schemata(cosmic_entropy_for_this_advancement, [](auto) {},
					[this, &session](const const_logic_step& step) {
					session.acquire_game_events_for_hud(step);
				}
			);

			session.resample_state_for_audiovisuals(hypersomnia);
		}

		const auto vdt = session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), timer);

		session.advance_audiovisual_systems(hypersomnia, testbed.get_selected_character(), vdt);

		std::wstring director_text = L"Welcome to the director setup";
		director_text += typesafe_sprintf(L"\nMode: %x", current_director_state == director_state::PLAYING ? "Playing" : "Recording");
		director_text += typesafe_sprintf(L"\nRequested playing speed: %x", requested_playing_speed);
		director_text += typesafe_sprintf(L"\nStep number: %x", hypersomnia.get_total_steps_passed());
		director_text += typesafe_sprintf(L"\nTime: %x", hypersomnia.get_total_time_passed_in_seconds());
		director_text += typesafe_sprintf(L"\nControlling entity %x of %x", testbed.current_character_index, testbed.characters.size());

		const auto director_log = augs::gui::text::format(director_text, augs::gui::text::style(assets::font_id::GUI_FONT, white));

		session.view(hypersomnia, testbed.get_selected_character(), window, vdt, director_log);
	}
}