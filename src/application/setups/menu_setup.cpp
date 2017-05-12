#include <thread>
#include <mutex>
#include <array>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/assets/assets_manager.h"

#include "game/test_scenes/testbed.h"
#include "game/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#include "augs/audio/sound_buffer.h"
#include "augs/audio/sound_source.h"

#include "augs/filesystem/file.h"
#include "augs/misc/action_list.h"
#include "augs/misc/standard_actions.h"
#include "menu_setup.h"

#include "augs/gui/text/caret.h"

#include "application/ui/app_ui_root.h"
#include "application/ui/app_ui_context.h"

#include "application/ui/appearing_text.h"

#include "augs/misc/http_requests.h"
#include "augs/templates/string_templates.h"

#include "augs/graphics/drawers.h"
#include "game/detail/visible_entities.h"
#include "application/config_lua_table.h"

#include "augs/audio/sound_samples_from_file.h"

using namespace augs::window::event::keys;
using namespace augs::gui::text;
using namespace augs::gui;

void menu_setup::process(
	const config_lua_table& cfg, 
	game_window& window,
	viewing_session& session
) {
	const auto metas_of_assets = get_assets_manager().generate_logical_metas_of_assets();

	const vec2i screen_size = vec2i(window.get_screen_size());

	cosmos intro_scene(3000);

	session.reserve_caches_for_entities(3000);
	session.show_profile_details = false;
	session.camera.averages_per_sec /= 2;

	session.drawing_settings.draw_gui_overlays = false;
	session.drawing_settings.draw_crosshairs = false;

	augs::single_sound_buffer menu_theme;
	augs::sound_source menu_theme_source;

	float gain_fade_multiplier = 0.f;

	if (cfg.music_volume > 0.f) {
		if (augs::file_exists(cfg.menu_theme_path)) {
			menu_theme.set_data(augs::get_sound_samples_from_file(cfg.menu_theme_path));

			menu_theme_source.bind_buffer(menu_theme);
			menu_theme_source.set_direct_channels(true);
			menu_theme_source.seek_to(static_cast<float>(cfg.start_menu_music_at_secs));
			menu_theme_source.set_gain(0.f);
			menu_theme_source.play();
		}
	}

	const style textes_style = style(assets::font_id::GUI_FONT, cyan);

	std::mutex news_mut;

	bool draw_cursor = false;
	bool roll_news = false;
	text_drawer latest_news_drawer;
	vec2 news_pos = vec2(static_cast<float>(screen_size.x), 5.f);

	std::thread latest_news_query([&latest_news_drawer, &cfg, &textes_style, &news_mut]() {
		auto result = augs::http_get_request(cfg.latest_news_url);
		const std::string delim = "newsbegin";

		const auto it = result.find(delim);

		if (it == std::string::npos) {
			return;
		}

		result = result.substr(it + delim.length());

		result = replace_all(result, "\n", "");
		result = replace_all(result, "\r", "");

		result = strip_tags(result, '<', '>');

		if (result.size() > 0) {
			const auto wresult = to_wstring(result);

			std::unique_lock<std::mutex> lck(news_mut);
			latest_news_drawer.set_text(format_as_bbcode(wresult, textes_style));
		}
	});

	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	timer.set_stepping_speed_multiplier(cfg.recording_replay_speed);

	test_scenes::testbed testbed;
	testbed.debug_var = cfg.debug_var;

	intro_scene.set_fixed_delta(cfg.default_tickrate);
	
	testbed.populate_world_with_entities(
		intro_scene, 
		metas_of_assets,
		session.get_standard_post_solve()
	);

	ltrb title_rect;
	title_rect.set_position({ 100, 100 });
	title_rect.set_size(metas_of_assets[assets::game_image_id::MENU_GAME_LOGO].get_size());

	rgba fade_overlay_color = { 0, 2, 2, 255 };
	rgba title_text_color = { 255, 255, 255, 0 };

	std::vector<appearing_text*> intro_texts;
	std::vector<appearing_text*> title_texts;

	rgba tweened_menu_button_color = cyan;
	tweened_menu_button_color.a = 0;

	vec2i tweened_welcome_message_bg_size;

	app_ui_rect_world menu_ui_rect_world;
	menu_ui_rect_world.last_state.screen_size = screen_size;
	app_ui_root menu_ui_root = app_ui_root(screen_size);
	app_ui_root_in_context menu_ui_root_id;

	auto center = [&](auto& t) {
		t.target_pos = screen_size / 2 - get_text_bbox(t.get_total_target_text(), 0)*0.5f;
	};

	appearing_text credits1;
	credits1.target_text[0] = format(L"hypernet community presents", textes_style);
	center(credits1);
	intro_texts.push_back(&credits1);

	appearing_text credits2;
	credits2.target_text = { format(L"A universe founded by\n", textes_style), format(L"Patryk B. Czachurski", textes_style) };
	center(credits2);
	intro_texts.push_back(&credits2);

	appearing_text developer_welcome;
	developer_welcome.population_interval = 60.f;

	developer_welcome.should_disappear = false;
	developer_welcome.target_text[0] = format(L"Thank you for building Hypersomnia.\n", textes_style);
	developer_welcome.target_text[1] = format(L"This message is not included in distributed executables.\n\
All your suggestions and contributions are very much welcomed by our collective.\n\
We wish you an exciting journey through architecture of our cosmos.\n", textes_style) +
format(L"    ~hypernet community", style(assets::font_id::GUI_FONT, { 0, 180, 255, 255 }));

	developer_welcome.target_pos += screen_size - get_text_bbox(developer_welcome.get_total_target_text(), 0) - vec2(100, 100);
	title_texts.push_back(&developer_welcome);

	appearing_text hypersomnia_description;
	hypersomnia_description.population_interval = 60.f;

	hypersomnia_description.should_disappear = false;
	hypersomnia_description.target_text[0] = format(L"- tendency of the omnipotent deity to immerse into inferior simulations,\nin spite of countless deaths experienced as a consequence.", { assets::font_id::GUI_FONT, {200, 200, 200, 255} });
	hypersomnia_description.target_pos = title_rect.right_top() + vec2(20, 20);
	title_texts.push_back(&hypersomnia_description);

	for (auto& m : menu_ui_root.menu_buttons) {
		m.hover_highlight_maximum_distance = 10.f;
		m.hover_highlight_duration_ms = 300.f;

		m.hover_sound.set_gain(cfg.sound_effects_volume);
		m.click_sound.set_gain(cfg.sound_effects_volume);
	}

	menu_ui_root.menu_buttons[menu_button_type::CONNECT_TO_OFFICIAL_UNIVERSE].set_appearing_caption(format(L"Login to\nofficial universe", textes_style));
	menu_ui_root.menu_buttons[menu_button_type::BROWSE_UNOFFICIAL_UNIVERSES].set_appearing_caption(format(L"Browse\nunofficial universes", textes_style));
	menu_ui_root.menu_buttons[menu_button_type::HOST_UNIVERSE].set_appearing_caption(format(L"Host\nuniverse", textes_style));
	menu_ui_root.menu_buttons[menu_button_type::CONNECT_TO_UNIVERSE].set_appearing_caption(format(L"Connect to\nuniverse", textes_style));
	menu_ui_root.menu_buttons[menu_button_type::LOCAL_UNIVERSE].set_appearing_caption(format(L"Local\nuniverse", textes_style));
	menu_ui_root.menu_buttons[menu_button_type::SETTINGS].set_appearing_caption(format(L"Settings", textes_style));
	menu_ui_root.menu_buttons[menu_button_type::CREATORS].set_appearing_caption(format(L"Creators", textes_style));
	menu_ui_root.menu_buttons[menu_button_type::QUIT].set_appearing_caption(format(L"Quit", textes_style));

	menu_ui_root.set_menu_buttons_positions(screen_size);

	vec2i tweened_menu_button_size;
	vec2i target_tweened_menu_button_size = menu_ui_root.get_max_menu_button_size();

	struct creators {
		struct entry {
			appearing_text task;
			std::vector<appearing_text> personae;

			void set_task(const formatted_string f) {
				task.target_text[0] = f;
				task.population_interval = 100.f;
				task.should_disappear = false;
			}

			void add_person(const formatted_string f) {
				appearing_text p;

				p.target_text[0] = f;
				p.population_interval = 100.f;
				p.should_disappear = false;

				personae.push_back(p);
			}

			vec2i get_personae_bbox() const {
				vec2i result;

				for (const auto& p : personae) {
					const auto p_bbox = get_text_bbox(p.get_total_target_text(), 0u);

					result.x = std::max(p_bbox.x, result.x);
					result.y += get_text_bbox(format(L"\n.", p.get_total_target_text()[0]), p_bbox.y).y;
				}

				return result;
			}
		};

		int personae_width = 0;
		int tasks_width = 0;
		int column_height = 0;

		std::vector<entry> entries;
		appearing_text afterword;

		void add_entry(entry e) {
			e.task.target_pos.set(0.f, column_height);

			const auto t_bbox = get_text_bbox(e.task.get_total_target_text(), 0u);

			tasks_width = std::max(t_bbox.x, tasks_width);

			for (auto& p : e.personae) {
				p.target_pos.set(0.f, column_height);

				const auto p_bbox = get_text_bbox(p.get_total_target_text(), 0u);

				personae_width = std::max(p_bbox.x, personae_width);
				column_height += get_text_bbox(format(L"\n.", p.get_total_target_text()[0]), p_bbox.y).y/2;
			}

			column_height += 15;

			entries.push_back(e);
		}

		void center_all(const vec2i screen_size) {
			for (auto& e : entries) {
				e.task.target_pos.y += screen_size.y / 2.f - column_height / 2.f;
				e.task.target_pos.x = screen_size.x / 2.f - tasks_width - 60.f;

				for (auto& p : e.personae) {
					p.target_pos.y += screen_size.y/2.f - column_height / 2.f;
					p.target_pos.x = screen_size.x / 2.f + 60.f;
				}
			}
		}

		void setup(const style s, const style task_st, const vec2i screen_size) {
			{
				entry c;

				c.set_task(format(L"Founder & Programmer", task_st));
				c.add_person(format(L"Patryk B. Czachurski", s));

				add_entry(c);
			}

			{
				entry c;

				c.set_task(format(L"Linux port", task_st));
				c.add_person(format(L"Adam Piekarczyk", s));

				add_entry(c);
			}

			{
				entry c;

				c.set_task(format(L"Pixel art", task_st));
				c.add_person(format(L"Michal Kawczynski", s));
				c.add_person(format(L"Patryk B. Czachurski", s));

				add_entry(c);
			}

			{
				entry c;

				c.set_task(format(L"Occasional helping hands", task_st));
				c.add_person(format(L"Bartosz P. Grzelak", s));

				add_entry(c);
			}

			center_all(screen_size);

			afterword.target_text[0] = format(L"\
What stands before your eyes is an outcome of a man's burning passion,\n\
a digital inamorata, chef d'oeuvre of a single coder, masterful musicians and a champion at pixel art.\n\n", s);

			afterword.target_text[1] = format(L"\
Its history of making recounts profound hopes,\n\
disillusions, crises and divine moments of joy.\n\
Cherish your ambitions for the immaterial.\n\
In the end you will either conquer that which you dreamed of,\n\
or tell a beautiful story of a man devastated by struggle.\n", s)
+ format(L"    ~Founder of the Hypersomnia Universe", task_st);

			afterword.target_pos.set(screen_size.x/2 - get_text_bbox(afterword.get_total_target_text(), 0u).x / 2, screen_size.y / 2 + column_height + 50);

			afterword.should_disappear = false;
			afterword.population_interval = 50.f;
			afterword.population_variation = 0.7f;
		}

		void push_into(augs::action_list& into) {
			size_t rng = 0;

			for (auto& e : entries) {
				augs::action_list entry_acts;
				
				augs::action_list task_acts;
				augs::action_list personae_acts;

				e.task.push_actions(task_acts, rng);

				for (auto& p : e.personae) {
					augs::action_list person_acts;

					p.push_actions(person_acts, rng);
					
					personae_acts.push_non_blocking(act(new augs::list_action(std::move(person_acts))));
				}

				entry_acts.push_non_blocking(act(new augs::list_action(std::move(task_acts))));
				entry_acts.push_non_blocking(act(new augs::list_action(std::move(personae_acts))));

				into.push_blocking(act(new augs::list_action(std::move(entry_acts))));
			}

			afterword.push_actions(into, rng);

			augs::action_list disappearance_acts;

			for (auto& e : entries) {
				e.task.push_disappearance(disappearance_acts, false);

				for (auto& p : e.personae) {
					p.push_disappearance(disappearance_acts, false);
				}
			}

			into.push_blocking(act(new augs::list_action(std::move(disappearance_acts))));

			afterword.push_disappearance(into);
		}

		void draw(augs::renderer& renderer) {
			for (auto& e : entries) {
				e.task.draw(renderer.get_triangle_buffer());

				for (auto& p : e.personae) {
					p.draw(renderer.get_triangle_buffer());
				}
			}

			afterword.draw(renderer.get_triangle_buffer());
		}
	};

	creators creators_screen;
	creators_screen.setup(textes_style, style(assets::font_id::GUI_FONT, { 0, 180, 255, 255 }), screen_size);

	augs::action_list intro_actions;
	augs::action_list credits_actions;

	{
		size_t rng = 0;

		if (!cfg.skip_credits) {
			intro_actions.push_blocking(act(new augs::delay_action(500.f)));
			intro_actions.push_non_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 100, 6000.f)));
			intro_actions.push_non_blocking(act(new augs::tween_value_action<float>(gain_fade_multiplier, 1.f, 6000.f)));
			intro_actions.push_blocking(act(new augs::delay_action(2000.f)));
			
			for (auto& t : intro_texts) {
				t->push_actions(intro_actions, rng);
			}
		}
		else {
			intro_actions.push_non_blocking(act(new augs::tween_value_action<float>(gain_fade_multiplier, 1.f, 6000.f)));
			fade_overlay_color.a = 100;
		}

		intro_actions.push_non_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 20, 500.f)));

		intro_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(tweened_menu_button_color.a, 255, 250.f)));
		{
			augs::action_list welcome_tweens;

			const auto bbox = get_text_bbox(developer_welcome.get_total_target_text(), 0);
			
			welcome_tweens.push_blocking(act(new augs::tween_value_action<int>(tweened_welcome_message_bg_size.x, bbox.x, 500.f)));
			welcome_tweens.push_blocking(act(new augs::tween_value_action<int>(tweened_welcome_message_bg_size.y, bbox.y, 350.f)));
			
			intro_actions.push_non_blocking(act(new augs::list_action(std::move(welcome_tweens))));
		}

		intro_actions.push_blocking(act(new augs::tween_value_action<int>(tweened_menu_button_size.x, target_tweened_menu_button_size.x, 500.f)));
		intro_actions.push_blocking(act(new augs::tween_value_action<int>(tweened_menu_button_size.y, target_tweened_menu_button_size.y, 350.f)));

		intro_actions.push_non_blocking(act(new augs::tween_value_action<rgba_channel>(title_text_color.a, 255, 500.f)));
		
		intro_actions.push_non_blocking(act(new augs::set_value_action<bool>(roll_news, true)));
		intro_actions.push_non_blocking(act(new augs::set_value_action<vec2i>(menu_ui_rect_world.last_state.mouse.pos, window.get_screen_size()/2)));
		intro_actions.push_non_blocking(act(new augs::set_value_action<bool>(draw_cursor, true)));
		
		for (auto& t : title_texts) {
			augs::action_list acts;
			t->push_actions(acts, rng);

#if !IS_PRODUCTION_BUILD
			intro_actions.push_non_blocking(act(new augs::list_action(std::move(acts))));
#endif
		}

		for (auto& m : menu_ui_root.menu_buttons) {
			augs::action_list acts;
			m.appearing_caption.push_actions(acts, rng);

			intro_actions.push_non_blocking(act(new augs::list_action(std::move(acts))));
		}
	}

	cosmic_movie_director director;
	director.load_recording_from_file(cfg.menu_intro_scene_entropy_path);
	ensure(director.is_recording_available());

	timer.reset_timer();

	const auto initial_step_number = intro_scene.get_total_steps_passed();

	auto menu_callback = [&](const menu_button_type t){
		switch (t) {

		case menu_button_type::QUIT:
			should_quit = true;
			break;

		case menu_button_type::CREATORS:
			if (credits_actions.is_complete()) {
				credits_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 170, 500.f)));
				creators_screen.push_into(credits_actions);
				credits_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 20, 500.f)));
			}
			break;

		default: break;
		}
	};

	while (intro_scene.get_total_time_passed_in_seconds() < cfg.rewind_intro_scene_by_secs) {
		const auto entropy = cosmic_entropy(director.get_entropy_for_step(intro_scene.get_total_steps_passed() - initial_step_number), intro_scene);

		intro_scene.advance_deterministic_schemata(
			{ entropy, metas_of_assets },
			[](auto) {},
			session.get_standard_post_solve()
		);
	}

	while (!should_quit) {
		augs::machine_entropy new_machine_entropy;

		{
			session.local_entropy_profiler.new_measurement();
			new_machine_entropy.local = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
			session.local_entropy_profiler.end_measurement();
			
			process_exit_key(new_machine_entropy.local);
		}

		auto steps = timer.count_logic_steps_to_perform(intro_scene.get_fixed_delta());

		while (steps--) {
			augs::renderer::get_current().clear_logic_lines();

			const auto entropy = cosmic_entropy(director.get_entropy_for_step(intro_scene.get_total_steps_passed() - initial_step_number), intro_scene);

			intro_scene.advance_deterministic_schemata(
				{ entropy, metas_of_assets },
				[](auto){},
				session.get_standard_post_solve()
			);
		}

		session.set_master_gain(cfg.sound_effects_volume * 0.3f * gain_fade_multiplier);
		menu_theme_source.set_gain(cfg.music_volume * gain_fade_multiplier);

		thread_local visible_entities all_visible;
		session.get_visible_entities(all_visible, intro_scene);

		const auto vdt = session.frame_timer.extract_variable_delta(
			intro_scene.get_fixed_delta(), 
			timer
		);

		session.advance_audiovisual_systems(
			intro_scene, 
			testbed.get_selected_character(), 
			all_visible,
			vdt
		);
		
		auto& renderer = augs::renderer::get_current();
		renderer.clear_current_fbo();

		const auto current_time_seconds = intro_scene.get_total_time_passed_in_seconds();

		session.view(
			cfg,
			renderer, 
			intro_scene,
			testbed.get_selected_character(),
			all_visible,
			timer.fraction_of_step_until_next_step(intro_scene.get_fixed_delta()), 
			augs::gui::text::format(typesafe_sprintf(L"Current time: %x", current_time_seconds), textes_style)
		);
		
		session.draw_color_overlay(renderer, fade_overlay_color);

		augs::draw_rect(
			renderer.get_triangle_buffer(),
			title_rect,
			assets::game_image_id::MENU_GAME_LOGO,
			title_text_color
		);

		if (tweened_welcome_message_bg_size.non_zero()) {
			augs::draw_rect_with_border(renderer.get_triangle_buffer(),
				ltrb(developer_welcome.target_pos, tweened_welcome_message_bg_size).expand_from_center(vec2(6, 6)),
				{ 0, 0, 0, 180 },
				slightly_visible_white);
		}

		for (auto& t : intro_texts) {
			t->draw(renderer.get_triangle_buffer());
		}

		for (auto& t : title_texts) {
			t->draw(renderer.get_triangle_buffer());
		}

		creators_screen.draw(renderer);

		if (roll_news) {
			news_pos.x -= vdt.in_seconds() * 100.f;

			{
				std::unique_lock<std::mutex> lck(news_mut);

				if (news_pos.x < -latest_news_drawer.get_bbox().x) {
					news_pos.x = static_cast<float>(screen_size.x);
				}

				latest_news_drawer.pos = news_pos;
				latest_news_drawer.draw_stroke(renderer.get_triangle_buffer());
				latest_news_drawer.draw(renderer.get_triangle_buffer());
			}
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		{
			app_ui_rect_world::gui_entropy gui_entropies;

			app_ui_rect_tree menu_ui_tree;
			app_ui_context menu_ui_context(menu_ui_rect_world, menu_ui_tree, menu_ui_root);

			menu_ui_rect_world.build_tree_data_into_context(menu_ui_context, menu_ui_root_id);

			if (draw_cursor) {
				for (const auto& ch : new_machine_entropy.local) {
					menu_ui_rect_world.consume_raw_input_and_generate_gui_events(menu_ui_context, menu_ui_root_id, ch, gui_entropies);
				}

				menu_ui_rect_world.call_idle_mousemotion_updater(menu_ui_context, menu_ui_root_id, gui_entropies);
			}

			menu_ui_rect_world.respond_to_events(menu_ui_context, menu_ui_root_id, gui_entropies);
			menu_ui_rect_world.advance_elements(menu_ui_context, menu_ui_root_id, vdt);

			menu_ui_root.set_menu_buttons_sizes(tweened_menu_button_size);
			menu_ui_root.set_menu_buttons_colors(tweened_menu_button_color);
			menu_ui_rect_world.rebuild_layouts(menu_ui_context, menu_ui_root_id);

			menu_ui_rect_world.draw(renderer.get_triangle_buffer(), menu_ui_context, menu_ui_root_id);

			for (size_t i = 0; i < menu_ui_root.menu_buttons.size(); ++i) {
				if (menu_ui_root.menu_buttons[i].click_callback_required) {
					menu_callback(static_cast<menu_button_type>(i));
					menu_ui_root.menu_buttons[i].click_callback_required = false;
				}
			}

			if (draw_cursor) {
				const auto mouse_pos = menu_ui_rect_world.last_state.mouse.pos;
				const auto gui_cursor_color = cyan;

				auto gui_cursor = assets::game_image_id::GUI_CURSOR;

				if (menu_ui_context.alive(menu_ui_rect_world.rect_hovered)) {
					gui_cursor = assets::game_image_id::GUI_CURSOR_HOVER;
				}

				augs::draw_rect(renderer.get_triangle_buffer(), mouse_pos, gui_cursor, gui_cursor_color);
			}

			renderer.call_triangles();
			renderer.clear_triangles();
		}

		intro_actions.update(vdt);
		credits_actions.update(vdt);

		window.swap_buffers();
	}

	latest_news_query.detach();
}