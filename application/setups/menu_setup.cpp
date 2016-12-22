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

#include "augs/gui/text/caret.h"

using namespace augs::window::event::keys;

void menu_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_rect());
	const auto& cfg = window.config;

	cosmos intro_scene(3000);
	
	augs::single_sound_buffer menu_theme;
	augs::sound_source menu_theme_source;

	if (cfg.music_volume > 0.f) {
		menu_theme.set_data(augs::get_sound_samples_from_file("hypersomnia/music/menu_theme.ogg"));

		menu_theme_source.bind_buffer(menu_theme);
		menu_theme_source.set_direct_channels(true);
		menu_theme_source.set_gain(cfg.music_volume);
		menu_theme_source.play();
	}

	using namespace augs::gui::text;

	augs::gui::text_drawer credits_drawer;
	const auto credits_style = style(assets::font_id::GUI_FONT, white);

	fstr credits_text;
	fstr target_credits_text;

	rgba fade_overlay_color = black;
	rgba credits_text_color;

	struct credits_entry {
		fstr text;
		fstr next_text;
	};

	credits_entry credits_texts[] = {
		{ format(L"hypernet community\npresents", credits_style) },
		{ format(L"A universe founded by\n", credits_style), format(L"Patryk B. Czachurski", credits_style) }
	};

	augs::action_list intro_actions;
	bool caret_active = false;

	{
		typedef std::unique_ptr<action> act;

		intro_actions.push_non_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 100, 6000.f)));
		intro_actions.push_blocking(act(new augs::delay_action(2000.f)));

		for (const auto& c : credits_texts) {
			const auto& text = c.text;

			intro_actions.push_blocking(act(new augs::set_value_action<rgba_channel>(credits_text_color.a, 255)));
			intro_actions.push_blocking(act(new augs::set_value_action<fstr>(target_credits_text, text + c.next_text)));
			intro_actions.push_blocking(act(new augs::set_value_action<fstr>(credits_text, fstr())));
			intro_actions.push_blocking(act(new augs::set_value_action<bool>(caret_active, true)));

			intro_actions.push_blocking(act(new augs::populate_with_delays<fstr>(credits_text, text, 150.f * text.length(), 0.4f)));

			if (c.next_text.size() > 0) {
				intro_actions.push_blocking(act(new augs::delay_action(1000.f)));
				intro_actions.push_blocking(act(new augs::populate_with_delays<fstr>(credits_text, c.next_text, 150.f * c.next_text.length(), 0.4f)));
			}

			intro_actions.push_blocking(act(new augs::delay_action(1000.f)));
			intro_actions.push_blocking(act(new augs::set_value_action<bool>(caret_active, false)));

			intro_actions.push_blocking(act(new augs::delay_action(1000.f)));
			intro_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(credits_text_color.a, 0, 2000.f)));
			intro_actions.push_blocking(act(new augs::delay_action(500.f)));
		}

		intro_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 0, 500.f)));
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

		if (credits_text.size() > 0) {
			credits_text = set_alpha(credits_text, credits_text_color.a);

			credits_drawer.pos = screen_size / 2 - get_text_bbox(target_credits_text, 0)*0.5f;
			credits_drawer.set_text(credits_text);
			
			caret_info in(credits_style);
			in.pos = credits_text.size();

			auto stroke_color = black;
			stroke_color.a = credits_text_color.a;
			credits_drawer.print.active = caret_active;
			credits_drawer.draw_stroke(renderer.get_triangle_buffer(), stroke_color);
			credits_drawer.draw(renderer.get_triangle_buffer(), &in);

			renderer.call_triangles();
			renderer.clear_triangles();
		}

		intro_actions.update(vdt);

		window.swap_buffers();
	}
}