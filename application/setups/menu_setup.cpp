#include <thread>
#include <array>
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
#include "augs/build_settings/setting_is_production_build.h"

#include "application/ui/app_ui_root.h"
#include "application/ui/app_ui_context.h"

using namespace augs::window::event::keys;
using namespace augs::gui::text;
using namespace augs::gui;

typedef std::unique_ptr<action> act;

struct appearing_text {
	text_drawer drawer;
	style st = style(assets::font_id::GUI_FONT, cyan);
	rgba_channel alpha;

	fstr text;
	
	std::array<fstr, 2> target_text;
	
	fstr get_total_target_text() const {
		return target_text[0] + target_text[1];
	}

	bool caret_active = false;
	bool should_disappear = true;
	bool blocking = true;

	vec2 target_pos;

	void push_actions(augs::action_list& into, size_t& rng) {
		auto push = [&](act a){
			blocking ? into.push_blocking(std::move(a)) : into.push_non_blocking(std::move(a));
		};

		push(act(new augs::set_value_action<rgba_channel>(alpha, 255)));
		push(act(new augs::set_value_action<fstr>(text, fstr())));
		push(act(new augs::set_value_action<bool>(caret_active, true)));

		push(act(new augs::populate_with_delays<fstr>(text, target_text[0], 150.f * target_text[0].length(), 0.4f, rng++)));

		if (target_text[1].size() > 0) {
			push(act(new augs::delay_action(1000.f)));
			push(act(new augs::populate_with_delays<fstr>(text, target_text[1], 150.f * target_text[1].length(), 0.4f, rng++)));
		}

		push(act(new augs::delay_action(1000.f)));
		push(act(new augs::set_value_action<bool>(caret_active, false)));

		if (should_disappear) {
			push(act(new augs::delay_action(1000.f)));
			push(act(new augs::tween_value_action<rgba_channel>(alpha, 0, 2000.f)));
			push(act(new augs::delay_action(500.f)));
		}
	}

	bool should_draw() const {
		return caret_active || (text.size() > 0 && alpha > 0);
	}

	void draw(augs::renderer& renderer) {
		if (!should_draw()) {
			return;
		}

		text = set_alpha(text, alpha);
		drawer.pos = target_pos;
		drawer.set_text(text);

		caret_info in(st);
		in.pos = text.size();

		auto stroke_color = black;
		stroke_color.a = alpha;
		drawer.print.active = caret_active;
		drawer.draw_stroke(renderer.get_triangle_buffer(), stroke_color);
		drawer.draw(renderer.get_triangle_buffer(), &in);
	}
};

void menu_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_size());
	const auto& cfg = window.config;

	cosmos intro_scene(3000);
	
	augs::single_sound_buffer menu_theme;
	augs::sound_source menu_theme_source;

	if (cfg.music_volume > 0.f) {
		const auto menu_theme_path = "hypersomnia/music/menu_theme.ogg";

		if (augs::file_exists(menu_theme_path)) {
			menu_theme.set_data(augs::get_sound_samples_from_file(menu_theme_path));

			menu_theme_source.bind_buffer(menu_theme);
			menu_theme_source.set_direct_channels(true);
			menu_theme_source.set_gain(cfg.music_volume);
			menu_theme_source.play();
		}
	}

	rgba fade_overlay_color = { 0, 2, 2, 255 };
	rgba title_text_color = { 255, 255, 255, 0 };

	const style textes_style = style(assets::font_id::GUI_FONT, cyan);

	std::vector<appearing_text*> texts;

	auto center = [&](auto& t) {
		t.target_pos = screen_size / 2 - get_text_bbox(t.get_total_target_text(), 0)*0.5f;
	};

	appearing_text credits1;
	credits1.target_text[0] = format(L"hypernet community presents", textes_style);
	center(credits1);
	texts.push_back(&credits1);

	appearing_text credits2;
	credits2.target_text = { format(L"A universe founded by\n", textes_style), format(L"Patryk B. Czachurski", textes_style) };
	center(credits2);
	texts.push_back(&credits2);

	appearing_text developer_welcome;
	developer_welcome.blocking = false;
	developer_welcome.should_disappear = false;
	developer_welcome.target_text[0] = format(L"Thank you for building Hypersomnia.\n", textes_style);
	developer_welcome.target_text[1] = format(L"This message is not included in distributed executables.\n\
All your suggestions and especially contributions are welcomed and sure to be considered.\n\
We wish you an exciting journey through architecture of our cosmos.\n\
                             ~hypernet community", textes_style);


	developer_welcome.target_pos += screen_size - get_text_bbox(developer_welcome.get_total_target_text(), 0) - vec2(100, 100);
	texts.push_back(&developer_welcome);

	augs::action_list intro_actions;

	vec2i tweened_menu_button_size = vec2i(80, 20);
	rgba tweened_menu_button_color = cyan;

	app_ui_rect_world menu_ui_rect_world;
	app_ui_rect_tree menu_ui_tree;
	app_ui_root menu_ui_root = app_ui_root(screen_size);
	app_ui_context menu_ui_context(menu_ui_rect_world, menu_ui_tree, menu_ui_root);
	app_ui_root_in_context menu_ui_root_id;

	{
		intro_actions.push_blocking(act(new augs::delay_action(500.f)));
		intro_actions.push_non_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 100, 6000.f)));
		intro_actions.push_blocking(act(new augs::delay_action(2000.f)));

		size_t rng = 0;

		credits1.push_actions(intro_actions, rng);
		credits2.push_actions(intro_actions, rng);

		intro_actions.push_non_blocking(act(new augs::tween_value_action<rgba_channel>(title_text_color.a, 255, 500.f)));
		intro_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 20, 500.f)));

#if !IS_PRODUCTION_BUILD
		developer_welcome.push_actions(intro_actions, rng);
#endif
	}

	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	scene_managers::testbed testbed;
	testbed.debug_var = window.config.debug_var;

	intro_scene.set_fixed_delta(cfg.tickrate);
	testbed.populate_world_with_entities(intro_scene, screen_size);
	const auto menu_title = intro_scene[testbed.get_menu_title_entity()];

	ltrb title_rect;
	title_rect.set_position(menu_title.logic_transform().pos);
	title_rect.set_size(menu_title.get<components::sprite>().size);

	viewing_session session;
	session.reserve_caches_for_entities(3000);
	session.camera.configure_size(screen_size);
	session.systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;
	session.show_profile_details = false;
	session.camera.averages_per_sec /= 2;

	cosmic_movie_director director;
	director.load_recording_from_file(cfg.menu_intro_scenario_filename);
	ensure(director.is_recording_available());

	testbed.configure_view(session);

	session.set_master_gain(cfg.sound_effects_volume * 0.1f);

	timer.reset_timer();

	const auto initial_step_number = intro_scene.get_total_steps_passed();

	while (!should_quit) {
		augs::machine_entropy new_entropy;

		{
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

		for (auto& t : texts) {
			t->draw(renderer);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		{
			app_ui_rect_world::gui_entropy gui_entropies;

			menu_ui_rect_world.build_tree_data_into_context(menu_ui_context, menu_ui_root_id);

			for (const auto& ch : new_entropy.local) {
				menu_ui_rect_world.consume_raw_input_and_generate_gui_events(menu_ui_context, menu_ui_root_id, ch, gui_entropies);
			}

			menu_ui_rect_world.call_idle_mousemotion_updater(menu_ui_context, menu_ui_root_id, gui_entropies);
			menu_ui_rect_world.advance_elements(menu_ui_context, menu_ui_root_id, gui_entropies, vdt);

			menu_ui_root.set_menu_buttons_sizes(tweened_menu_button_size);
			menu_ui_root.set_menu_buttons_colors(tweened_menu_button_color);
			menu_ui_rect_world.rebuild_layouts(menu_ui_context, menu_ui_root_id);

			menu_ui_rect_world.draw(renderer.get_triangle_buffer(), menu_ui_context, menu_ui_root_id);

			renderer.call_triangles();
			renderer.clear_triangles();
		}

		intro_actions.update(vdt);
		intro_actions.update(vdt);
		intro_actions.update(vdt);

		menu_title.get<components::sprite>().color = title_text_color;

		window.swap_buffers();
	}
}