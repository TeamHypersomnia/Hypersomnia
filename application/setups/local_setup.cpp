#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/one_entity.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_messages_includes.h"

#include "augs/misc/machine_entropy_player.h"

#include "augs/filesystem/file.h"
#include "local_setup.h"

using namespace augs::window::event::keys;

void local_setup::process(const config_lua_table& cfg, game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_size());

	cosmos hypersomnia(3000);

	augs::machine_entropy total_collected_entropy;
	augs::machine_entropy_player player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	scene_managers::testbed testbed;
	testbed.debug_var = cfg.debug_var;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(cfg.tickrate);
		testbed.populate_world_with_entities(hypersomnia, screen_size);
	}

	if (cfg.get_input_recording_mode() != input_recording_type::DISABLED) {
		if (player.try_to_load_or_save_new_session("sessions/", "recorded.inputs")) {
			timer.set_stepping_speed_multiplier(cfg.recording_replay_speed);
		}
	}

	viewing_session session;
	session.reserve_caches_for_entities(3000);
	session.camera.configure_size(screen_size);
	session.systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;
	session.set_master_gain(cfg.sound_effects_volume);

	session.configure_input();

	cfg.update_configuration_for_entity(hypersomnia[testbed.get_selected_character()]);

	timer.reset_timer();

	const bool debug_control_timing = false;

	while (!should_quit) {
		{
			augs::machine_entropy new_entropy;

			session.local_entropy_profiler.new_measurement();
			new_entropy.local = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
			session.local_entropy_profiler.end_measurement();
			
			session.control(new_entropy);

			process_exit_key(new_entropy.local);

			total_collected_entropy += new_entropy;

			if (debug_control_timing) {
				for (const auto& raw_input : new_entropy.local) {
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

			for (const auto& raw_input : new_entropy.local) {
				if (raw_input.was_any_key_pressed()) {
					if (raw_input.key == key::F2) {
						LOG_COLOR(console_color::YELLOW, "Separator");
					}
				}
			}
		}

		auto steps = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());

		while (steps--) {
			player.advance_player_and_biserialize(total_collected_entropy);

			if (debug_control_timing) {
				for (const auto& raw_input : total_collected_entropy.local) {
					if (raw_input.was_any_key_pressed()) {
						if (raw_input.key == key::_1) {
							hypersomnia.set_fixed_delta(cfg.tickrate);
						}
						if (raw_input.key == key::_2) {
							hypersomnia.set_fixed_delta(128);
						}
						if (raw_input.key == key::_3) {
							hypersomnia.set_fixed_delta(144);
						}
					}
				}
			}
			
			testbed.control_character_selection(total_collected_entropy.local);

			const auto cosmic_entropy_for_this_step = cosmic_entropy(hypersomnia[testbed.get_selected_character()], total_collected_entropy.local, session.context);

			augs::renderer::get_current().clear_logic_lines();

			hypersomnia.advance_deterministic_schemata(cosmic_entropy_for_this_step, [](auto){},
				[this, &session](const const_logic_step step){
					session.acquire_game_events_for_hud(step);
				}
			);

			session.resample_state_for_audiovisuals(hypersomnia);
			
			total_collected_entropy.clear();
		}

		const auto vdt = session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), timer);

		session.advance_audiovisual_systems(hypersomnia, testbed.get_selected_character(), vdt);

		auto& renderer = augs::renderer::get_current();
		renderer.clear_current_fbo();

		game_drawing_settings settings;
		settings.draw_weapon_laser = true;

		session.view(renderer, hypersomnia, testbed.get_selected_character(), vdt, augs::gui::text::fstr(), settings);

		window.swap_buffers();
	}
}