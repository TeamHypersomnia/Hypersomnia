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
#include "game/transcendental/game_drawing_settings.h"

#include "augs/misc/machine_entropy_player.h"

#include "augs/audio/sound_buffer.h"
#include "augs/audio/sound_source.h"

#include "augs/filesystem/file.h"
#include "augs/misc/action_list.h"
#include "augs/misc/standard_actions.h"
#include "menu_setup.h"

using namespace augs::window::event::keys;

void menu_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_rect());
	const auto& cfg = window.config;

	cosmos intro_scene(3000);
	
	if (cfg.music_volume > 0.f) {
		augs::single_sound_buffer menu_theme;
		menu_theme.set_data(augs::get_sound_samples_from_file("hypersomnia/music/menu_theme.ogg"));

		augs::sound_source menu_theme_source;
		menu_theme_source.bind_buffer(menu_theme);
		menu_theme_source.set_direct_channels(true);
		menu_theme_source.set_gain(cfg.music_volume);
		menu_theme_source.play();
	}

	rgba fade_overlay_color = black;

	augs::action_list intro_actions;
	std::unique_ptr<action> ac(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 0, 3000.f));

	{
		intro_actions.push_non_blocking(std::unique_ptr<action>(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 70, 5000.f)));
	}

	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	scene_managers::testbed testbed;
	testbed.debug_var = window.config.debug_var;

	intro_scene.set_fixed_delta(cfg.tickrate);
	testbed.populate_world_with_entities(intro_scene, screen_size);

	viewing_session session;
	session.reserve_caches_for_entities(3000);
	session.camera.configure_size(screen_size);
	session.systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;
	session.show_profile_details = false;

	cosmic_movie_director director;
	director.load_recording_from_file(cfg.menu_intro_scenario_filename);
	ensure(director.is_recording_available());

	testbed.configure_view(session);

	session.set_master_gain(cfg.sound_effects_volume * 0.1f);

	timer.reset_timer();

	const auto initial_step_number = intro_scene.get_total_steps_passed();

	while (!should_quit) {
		{
			augs::machine_entropy new_entropy;

			session.local_entropy_profiler.new_measurement();
			new_entropy.local = window.collect_entropy();
			session.local_entropy_profiler.end_measurement();
			
			session.control(new_entropy);

			process_exit_key(new_entropy.local);
		}

		auto steps = timer.count_logic_steps_to_perform(intro_scene.get_fixed_delta());

		while (steps--) {
			renderer::get_current().clear_logic_lines();

			const auto entropy = cosmic_entropy(director.get_entropy_for_step(intro_scene.get_total_steps_passed() + initial_step_number), intro_scene);

			intro_scene.advance_deterministic_schemata(entropy, [](auto){},
				[this, &session](const const_logic_step& step){
					session.acquire_game_events_for_hud(step);
				}
			);

			session.resample_state_for_audiovisuals(intro_scene);
		}

		const auto vdt = session.frame_timer.extract_variable_delta(intro_scene.get_fixed_delta(), timer);

		session.advance_audiovisual_systems(intro_scene, testbed.get_selected_character(), vdt);

		auto& renderer = augs::renderer::get_current();
		renderer.clear_current_fbo();

		game_drawing_settings settings;
		settings.draw_gui_overlays = false;
		settings.draw_crosshairs = false;

		session.view(renderer, intro_scene, testbed.get_selected_character(), vdt, augs::gui::text::fstr(), settings);
		session.draw_color_overlay(renderer, fade_overlay_color);

		intro_actions.update(vdt);

		window.swap_buffers();
	}
}