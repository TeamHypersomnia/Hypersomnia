#include "viewing_session.h"
#include "augs/window_framework/window.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_step.h"
#include "game/view/rendering_scripts/all.h"
#include "augs/misc/machine_entropy.h"
#include "game/components/flags_component.h"
#include "game/messages/item_picked_up_message.h"

#include "augs/network/network_client.h"
#include "hypersomnia_version.h"
#include "game/build_settings.h"

#include "augs/misc/lua_readwrite.h"
#include "generated/introspectors.h"
#include "augs/filesystem/file.h"
#include "augs/misc/imgui_utils.h"
#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/hardcoded_content/test_scenes/testbed.h"
#include "game/hardcoded_content/test_scenes/minimal_scene.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#if ONLY_ONE_GLOBAL_ASSETS_MANAGER
assets_manager global_instance;
#endif

debug_drawing_settings DEBUG_DRAWING;

viewing_session::viewing_session(
	const config_lua_table& passed_config,
	const std::string& config_path_for_saving
) : 
	config_path_for_saving(config_path_for_saving),
	config(passed_config),
	last_saved_config(config),
	window(config.window),
	gl(config.window.get_screen_size()),
	audio_context(config.audio)
{
	const auto screen_size = config.window.get_screen_size();
	set_screen_size(screen_size);

	gl.set_as_current();

	auto& io = ImGui::GetIO();
	
	using namespace augs::event::keys;

	auto map_key = [&io](auto im, auto aug) {
		io.KeyMap[im] = int(aug);
	};

	map_key(ImGuiKey_Tab, key::TAB);
	map_key(ImGuiKey_LeftArrow, key::LEFT);
	map_key(ImGuiKey_RightArrow, key::RIGHT);
	map_key(ImGuiKey_UpArrow, key::UP);
	map_key(ImGuiKey_DownArrow, key::DOWN);
	map_key(ImGuiKey_PageUp, key::PAGEUP);
	map_key(ImGuiKey_PageDown, key::PAGEDOWN);
	map_key(ImGuiKey_Home, key::HOME);
	map_key(ImGuiKey_End, key::END);
	map_key(ImGuiKey_Delete, key::DEL);
	map_key(ImGuiKey_Backspace, key::BACKSPACE);
	map_key(ImGuiKey_Enter, key::ENTER);
	map_key(ImGuiKey_Escape, key::ESC);
	map_key(ImGuiKey_A, key::A);
	map_key(ImGuiKey_C, key::C);
	map_key(ImGuiKey_V, key::V);
	map_key(ImGuiKey_X, key::X);
	map_key(ImGuiKey_Y, key::Y);
	map_key(ImGuiKey_Z, key::Z);

	io.IniFilename = "generated/imgui.ini";
	io.LogFilename = "generated/imgui_log.txt";
	io.MouseDoubleClickMaxDist = 100.f;

	ImGui::GetStyle() = config.gui_style;

#if !ONLY_ONE_GLOBAL_ASSETS_MANAGER
	auto resources = std::make_unique<assets_manager>();
	resources->set_as_current();
#endif

	auto& resources = get_assets_manager();

	load_all_requisite(global_instance, config.content_regeneration);

	load_requisite_atlases(resources);
	load_requisite_shaders(resources);

	ingame_menu_ui_context::root_type ingame_ui_root(screen_size);

	const auto gui_text_style = augs::gui::text::style(assets::font_id::GUI_FONT, cyan);

	for (std::size_t i = 0; i < ingame_ui_root.menu_buttons.size(); ++i) {
		const auto e = static_cast<ingame_menu_button_type>(i);
		ingame_ui_root.menu_buttons[i].set_complete_caption(augs::gui::text::format(to_wstring(format_enum(e)), gui_text_style));
	}

	ingame_menu_ui_context::rect_world_type ingame_ui_world(screen_size);

	main_menu_ui_context::root_type main_menu_ui_root(screen_size);

	for (std::size_t i = 0; i < main_menu_ui_root.menu_buttons.size(); ++i) {
		const auto e = static_cast<main_menu_button_type>(i);
		main_menu_ui_root.menu_buttons[i].set_complete_caption(augs::gui::text::format(to_wstring(format_enum(e)), gui_text_style));
	}

	auto make_menu_gui_complete = [&main_menu_ui_root]() {
		for (auto& m : main_menu_ui_root.menu_buttons) {
			m.make_complete();
		}
	};

	main_menu_ui_context::rect_world_type main_menu_ui_world(screen_size);

	const auto mode = config.get_launch_mode();
	LOG("Launch mode: %x", static_cast<int>(mode));

	ensure
	(
		(
			mode == launch_type::LOCAL
			|| mode == launch_type::MAIN_MENU
		)
		&& "The launch mode you have chosen is currently out of service."
	);

	auto emplace_setup = [&](auto&&... args) {
		current_setup.emplace(std::forward<decltype(args)>(args)...);
		menu.reset();
	};

	auto test_scene_populate = [&](cosmos& world, auto& metas_of_assets) {
		metas_of_assets = *get_assets_manager().generate_logical_metas_of_assets();

		world.reserve_storage_for_entities(3000u);
		audiovisuals.reserve_caches_for_entities(3000u);

		if (config.debug.create_minimal_test_scene) {
			test_scenes::minimal_scene().populate_world_with_entities(
				world,
				metas_of_assets,
				get_standard_post_solve()
			);
		}
		else {
			test_scenes::testbed().populate_world_with_entities(
				world,
				metas_of_assets,
				get_standard_post_solve()
			);
		}
	};


	if (mode == launch_type::MAIN_MENU) {
		menu.emplace(test_scene_populate);

		// play intro if not skipped
		if (config.main_menu.skip_credits) {
			make_menu_gui_complete();
		}
	}
	else if (mode == launch_type::LOCAL) {
		emplace_setup(std::in_place_type_t<local_setup>(),
			test_scene_populate,
			config.get_input_recording_mode()
		);
	}

	auto& renderer = gl;

	bool should_quit = false;

	auto get_character_gui_mouse_pos = [&]() -> vec2i& {
		return audiovisuals.get<gui_element_system>().rect_world.last_state.mouse.pos;
	};

	while (!should_quit) {
		const auto screen_size = config.window.get_screen_size();

		auto scope = measure_scope(profiler.fps);

		augs::machine_entropy new_machine_entropy;

		{
			auto scope = measure_scope(profiler.local_entropy);
			new_machine_entropy.local = window.collect_entropy();
		}

		perform_imgui_pass(
			new_machine_entropy.local,
			static_cast<float>(imgui_timer.extract<std::chrono::milliseconds>())
		);

		using namespace augs::event;

		bool release_keys = false;
		bool release_mouse = false;

		for (const auto& n : new_machine_entropy.local) {
			state.apply(n);

			if (
				n.msg == message::close
				|| n.msg == message::quit
				|| (n.msg == message::syskeydown && n.key.key == keys::key::F4)
			) {
				should_quit = true;
			}

			if (n.msg == message::keydown && n.key.key == keys::key::ESC) {
				if (current_setup.has_value()) {
					show_ingame_menu = !show_ingame_menu;
					release_keys = release_mouse = true;
				}
			}

			if (n.msg == message::activate) {
				release_keys = release_mouse = true;
			}
		}

		if (!show_ingame_menu && switch_between_gui_and_back(new_machine_entropy.local)) {
			release_mouse = true;
		}

		thread_local bool released_due_to_text_input = false;

		if (ImGui::GetIO().WantTextInput) {
			if (!released_due_to_text_input) {
				release_keys = release_mouse = true;
				released_due_to_text_input = true;
			}
		}
		else {
			released_due_to_text_input = false;
		}

		auto apply_releases = [&](const auto& released) {
			for (const auto c : released) {
				this->state.apply(c);
			}

			concatenate(new_machine_entropy.local, released);
		};

		if (release_keys) {
			apply_releases(state.generate_key_releasing_changes());
		}

		if (release_mouse) {
			apply_releases(state.generate_mouse_releasing_changes());
		}

		//if (ImGui::GetIO().WantCaptureMouse) {
			// avoid cursor hopping
			ingame_ui_world.last_state.mouse.pos = ImGui::GetIO().MousePos;
			main_menu_ui_world.last_state.mouse.pos = ImGui::GetIO().MousePos;
			audiovisuals.get<gui_element_system>().rect_world.last_state.mouse.pos = ImGui::GetIO().MousePos;
		//}

		renderer.set_viewport({ { 0, 0 }, screen_size });

		renderer.clear_current_fbo();
		auto& output = renderer.get_triangle_buffer();

		if (current_setup.has_value()) {
			std::visit(
				[&](auto& setup){
					thread_local visible_entities all_visible;
					
					using T = std::decay_t<decltype(setup)>;

					if constexpr(std::is_same_v<T, local_setup>) {
						const auto selected_character = setup.get_selected_character();
						const auto& viewed_cosmos = setup.get_viewing_cosmos();

						const bool fetch_last_time = release_keys || release_mouse;

						if (!show_ingame_menu || fetch_last_time) {
							fetch_gui_events(
								selected_character,
								new_machine_entropy.local
							);

							setup.control(
								new_machine_entropy.local,
								config.controls
							);
							
							setup.control(
								audiovisuals.get<gui_element_system>().get_and_clear_pending_events()
							);
						}

						auto translated = config.controls.translate(new_machine_entropy.local);
						fetch_developer_console_intents(translated.intents);

						if (!show_ingame_menu) {
							fetch_session_intents(translated.intents);
						}
						
						setup.advance(
							[&](){
								get_visible_entities(all_visible, viewed_cosmos);

								audiovisuals.advance({
									viewed_cosmos,
									selected_character,
									all_visible,
									static_cast<float>(setup.get_audiovisual_speed()),

									config.audio_volume,
									config.interpolation,
									config.camera
								});
							},

							get_standard_post_solve()
						);

						view(
							renderer,
							viewed_cosmos,
							selected_character
							all_visible,
							setup.get_interpolation_ratio()
						);
					}
				}, 
				current_setup.value()
			);

			if (show_ingame_menu) {
				draw_color_overlay(gl.get_triangle_buffer(), { 0, 0, 0, 140 });

				const auto vdt = augs::delta(
					static_cast<float>(
						gui_timer.extract<std::chrono::milliseconds>()
					)
				);

				auto& root = ingame_ui_root;
				auto& world = ingame_ui_world;

				root.set_screen_size(screen_size);
				world.set_screen_size(screen_size);

				auto tree = ingame_menu_ui_context::tree_type();
				auto context = ingame_menu_ui_context(config.audio_volume, world, tree, root);

				root.set_menu_buttons_sizes({ 1000, 1000 });

				world.build_tree_data_into(context);

				const auto gui_entropies = augs::consume_inputs_with_imgui_sync(context, new_machine_entropy.local);

				world.respond_to_events(context, gui_entropies);
				world.advance_elements(context, vdt);

				root.set_menu_buttons_colors(cyan);
				world.rebuild_layouts(context);

				root.draw_background_behind_buttons(output);

				world.draw(output, context);

				renderer.call_triangles();
				renderer.clear_triangles();

				renderer.draw_call_imgui(resources);
				
				auto gui_cursor = assets::game_image_id::GUI_CURSOR;
				
				if (context.alive(world.rect_hovered)) {
					gui_cursor = assets::game_image_id::GUI_CURSOR_HOVER;
				}

				if (ImGui::GetIO().WantCaptureMouse) {
					gui_cursor = augs::get_imgui_cursor<assets::game_image_id>();
				}

				const vec2i cursor_drawing_pos = ImGui::GetIO().MousePos;
				augs::draw_cursor(output, cursor_drawing_pos, gui_cursor, white);

				auto clicked = [&root](const ingame_menu_button_type t) {
					if (root.menu_buttons[t].click_callback_required) {
						root.menu_buttons[t].click_callback_required = false;
						return true;
					}

					return false;
				};

				if (clicked(ingame_menu_button_type::RESUME)) {
					show_ingame_menu = false;
				}

				if (clicked(ingame_menu_button_type::LEAVE_THIS_UNIVERSE)) {
					show_ingame_menu = false;
					current_setup.reset();
				}

				if (clicked(ingame_menu_button_type::QUIT)) {
					should_quit = true;
				}

				if (clicked(ingame_menu_button_type::SETTINGS)) {
					show_settings = true;
					ImGui::SetWindowFocus("Settings");
				}

				const bool wont_receive_events_anymore = !show_ingame_menu;

				if (wont_receive_events_anymore) {
					/* We must manually unhover "Resume" button */
					ingame_menu_ui_context::rect_world_type::gui_entropy gui_entropies;

					world.unhover_and_undrag(context, gui_entropies);
					world.respond_to_events(context, gui_entropies);
				}

				renderer.call_triangles();
				renderer.clear_triangles();
			}
			else {
				auto& gui = audiovisuals.get<gui_element_system>();

				renderer.draw_call_imgui(resources);

				//game_gui_rect_tree tree;
				//
				//const auto context = gui.create_context(
				//	std::visit(
				//		[&](auto& setup) {
				//			return setup.get_selected_character();
				//		},
				//		*current_setup
				//	), 
				//	tree
				//);
				//
				//const vec2i cursor_drawing_pos = ImGui::GetIO().MousePos;
				//draw_gui_cursor(context.alive(context.get_rect_world().rect_hovered), output, cursor_drawing_pos, white);
				

				if (ImGui::GetIO().WantCaptureMouse) {
					const vec2i cursor_drawing_pos = ImGui::GetIO().MousePos;
					augs::draw_cursor(output, cursor_drawing_pos, augs::get_imgui_cursor<assets::game_image_id>(), white);

					renderer.call_triangles();
					renderer.clear_triangles();
				}
				else {
					if (gui.gui_look_enabled) {
						const auto controlled_entity = std::visit(
							[&](auto& setup) {
							return setup.get_selected_character();
						}, *current_setup);

						const auto ratio = std::visit(
							[&](auto& setup) {
							return setup.get_interpolation_ratio();
						}, *current_setup);

						gui.get_character_gui(controlled_entity).draw_cursor_with_information(
						{
							gui,
							audiovisuals.get<interpolation_system>(),
							controlled_entity,
							audiovisuals.world_hover_highlighter,
							config.hotbar,
							ratio,
							config.controls,
							audiovisuals.camera.smoothed_camera,
							output
						});

						renderer.call_triangles();
						renderer.clear_triangles();
					}
				}
			}
		}
		else {
			thread_local visible_entities all_visible;

			show_ingame_menu = false;

			if (!menu.has_value()) {
				menu.emplace(test_scene_populate);
				audiovisuals = decltype(audiovisuals)();
				make_menu_gui_complete();
			}

			const auto vdt = augs::delta(
				static_cast<float>(
					gui_timer.extract<std::chrono::milliseconds>()
				)
			);

			auto& root = main_menu_ui_root;
			auto& world = main_menu_ui_world;

			root.set_screen_size(screen_size);
			world.set_screen_size(screen_size);

			auto tree = main_menu_ui_context::tree_type();
			auto context = main_menu_ui_context(config.audio_volume, world, tree, root);

			root.set_menu_buttons_sizes({ 1000, 1000 });

			world.build_tree_data_into(context);

			const auto gui_entropies = augs::consume_inputs_with_imgui_sync(context, new_machine_entropy.local);

			world.respond_to_events(context, gui_entropies);
			world.advance_elements(context, vdt);

			root.set_menu_buttons_colors(cyan);
			world.rebuild_layouts(context);

			root.draw_background_behind_buttons(output);

			world.draw(output, context);
			
			renderer.set_standard_blending();

			{
				auto& shader = resources.at(assets::shader_program_id::DEFAULT);
				shader.set_as_current();
				shader.set_projection(screen_size);
			}

			resources.at(assets::gl_texture_id::GAME_WORLD_ATLAS).bind();

			renderer.call_triangles();
			renderer.clear_triangles();

			renderer.draw_call_imgui(resources);
			
			auto gui_cursor = assets::game_image_id::GUI_CURSOR;

			if (context.alive(world.rect_hovered)) {
				gui_cursor = assets::game_image_id::GUI_CURSOR_HOVER;
			}

			if (ImGui::GetIO().WantCaptureMouse) {
				gui_cursor = augs::get_imgui_cursor<assets::game_image_id>();
			}

			const vec2i cursor_drawing_pos = ImGui::GetIO().MousePos;
			augs::draw_cursor(output, cursor_drawing_pos, gui_cursor, white);

			auto clicked = [&root](const main_menu_button_type t) {
				if (root.menu_buttons[t].click_callback_required) {
					root.menu_buttons[t].click_callback_required = false;
					return true;
				}

				return false;
			};

			if (clicked(main_menu_button_type::SETTINGS)) {
				show_settings = true;
				ImGui::SetWindowFocus("Settings");
			}

			if (clicked(main_menu_button_type::LOCAL_UNIVERSE)) {
				emplace_setup(std::in_place_type_t<local_setup>(),
					test_scene_populate,
					config.get_input_recording_mode()
				);
			}

			if (clicked(main_menu_button_type::QUIT)) {
				should_quit = true;
			}

			renderer.call_triangles();
			renderer.clear_triangles();
		}

		window.swap_buffers();
	}

	audiovisuals.get<sound_system>() = sound_system();
	main_menu_ui_root = decltype(main_menu_ui_root)({});
	ingame_ui_root = decltype(ingame_ui_root)({});

	//	By now, all sound sources from the viewing session are released.

#if ONLY_ONE_GLOBAL_ASSETS_MANAGER
	/*
	We need to manually destroy the global assets manager,
	before the audio manager gets destroyed.
	*/
	resources = assets_manager();
#endif
}

#include "application/menu_ui/menu_ui_context.h"

void viewing_session::perform_ingame_menu() {


}

void viewing_session::sync_back(config_lua_table& into) {
	into.window = window.get_current_settings();
}

void viewing_session::apply(
	const config_lua_table& new_config
) {
	const auto settings = new_config.get_viewing_session_settings();
	gl.resize_fbos(settings.screen_size);
	set_screen_size(settings.screen_size);

	DEBUG_DRAWING = new_config.debug_drawing;
	window.apply(new_config.window);
	audio_context.apply(new_config.audio);
}

void viewing_session::set_screen_size(const vec2i new_size) {
	audiovisuals.set_screen_size(new_size);
	gl.resize_fbos(new_size);
}

bool viewing_session::switch_between_gui_and_back(const augs::machine_entropy::local_type& local) {
	auto& gui = audiovisuals.get<gui_element_system>();

	bool has_changed = false;

	for (const auto& intent : config.controls.translate(local).intents) {
		if (intent.is_pressed && intent.intent == intent_type::SWITCH_TO_GUI) {
			gui.gui_look_enabled = !gui.gui_look_enabled;
			has_changed = true;
		}
	}

	return has_changed;
}

void viewing_session::fetch_gui_events(
	const const_entity_handle root,
	augs::machine_entropy::local_type& window_inputs
) {
	if (root.alive()) {
		auto& gui = audiovisuals.get<gui_element_system>();

		gui.control_gui(
			root, 
			window_inputs
		);

		gui.handle_hotbar_and_action_button_presses(
			root,
			config.controls.translate(window_inputs).intents
		);
	}
}

void viewing_session::perform_imgui_pass(
	augs::machine_entropy::local_type& window_inputs,
	const augs::delta dt
) {
	const auto size = audiovisuals.camera.camera.visible_world_area;

	auto& io = ImGui::GetIO();

	using namespace augs::event;
	using namespace augs::event::keys;
	
	io.MouseDrawCursor = false;

	for (const auto& in : window_inputs) {
		if (in.msg == message::mousemotion) {
			io.MousePos = vec2(in.mouse.pos);
		}
		else if (in.msg == message::ldown || in.msg == message::ldoubleclick || in.msg == message::ltripleclick) {
			io.MouseDown[0] = true;
		}
		else if (in.msg == message::lup) {
			io.MouseDown[0] = false;
		}
		else if (in.msg == message::rdown || in.msg == message::rdoubleclick) {
			io.MouseDown[1] = true;
		}
		else if (in.msg == message::rup) {
			io.MouseDown[1] = false;
		}
		else if (in.msg == message::wheel) {
			io.MouseWheel = static_cast<float>(in.scroll.amount);
		}
		else if (in.msg == message::keydown) {
			io.KeysDown[int(in.key.key)] = true;
		}
		else if (in.msg == message::keyup) {
			io.KeysDown[int(in.key.key)] = false;
		}
		else if (in.msg == message::character) {
			io.AddInputCharacter(in.character.utf16);
		}

		io.KeyCtrl = io.KeysDown[int(keys::key::LCTRL)] || io.KeysDown[int(keys::key::RCTRL)];
		io.KeyShift = io.KeysDown[int(keys::key::LSHIFT)] || io.KeysDown[int(keys::key::RSHIFT)];
		io.KeyAlt = io.KeysDown[int(keys::key::LALT)] || io.KeysDown[int(keys::key::RALT)];
	}
	
	const bool in_game_without_mouse =
		current_setup.has_value()
		&& !audiovisuals.get<gui_element_system>().gui_look_enabled
		&& !show_ingame_menu
	;

	if (in_game_without_mouse) {
		io.MousePos = { -1, -1 };
		
		for (auto& m : io.MouseDown) {
			m = false;
		}
	}

	window.set_mouse_position_frozen(in_game_without_mouse);

	io.DeltaTime = dt.in_seconds();
	io.DisplaySize = size;

	ImGui::NewFrame();

	sync_back(config);
	perform_settings_gui();
	apply(config);

	ImGui::Render();

	window_inputs = augs::filter_inputs_for_imgui(window_inputs);
}

void viewing_session::fetch_developer_console_intents(game_intent_vector& intents) {
	erase_if(intents, [&](const game_intent& intent) {
		bool fetch = false;

		if (intent.intent == intent_type::OPEN_DEVELOPER_CONSOLE) {
			fetch = true;

			if (intent.is_pressed) {
				config.debug.show_developer_console = !config.debug.show_developer_console;
			}
		}

		return fetch;
	});
}

void viewing_session::fetch_session_intents(game_intent_vector& intents) {
	erase_if(intents, [&](const game_intent& intent) {
		bool fetch = false;

		if (intent.intent == intent_type::CLEAR_DEBUG_LINES) {
			augs::renderer::get_current().persistent_lines.lines.clear();
		}
		else if (intent.intent == intent_type::SWITCH_WEAPON_LASER) {
			fetch = true;

			if (intent.is_pressed) {
				config.drawing.draw_weapon_laser = !config.drawing.draw_weapon_laser;
			}
		}

		return fetch;
	});
}

void viewing_session::draw_text_at_left_top(
	augs::renderer& renderer,
	const augs::gui::text::formatted_string& str
) const {
	quick_print(renderer.triangles, str, vec2i(0, 0), 0);
}

std::wstring viewing_session::get_profile_summary() const {
	return profiler.summary();
}

void viewing_session::view(
	augs::renderer& renderer,
	const cosmos& cosmos,
	const entity_id viewed_character,
	const visible_entities& all_visible,
	const double interpolation_ratio,
	const augs::gui::text::formatted_string& custom_log
) const {
	auto frame = measure_scope(profiler.frame);

	const vec2i screen_size = audiovisuals.camera.smoothed_camera.visible_world_area;

	const auto character_chased_by_camera = cosmos[viewed_character];

	auto main_cosmos_viewing_step = viewing_step(
		cosmos, 
		audiovisuals,
		config.drawing,
		config.hotbar,
		config.controls,
		interpolation_ratio,
		renderer, 
		audiovisuals.camera.smoothed_camera,
		character_chased_by_camera,
		all_visible,
		show_ingame_menu
	);

	rendering_scripts::illuminated_rendering(main_cosmos_viewing_step);

	using namespace augs::gui::text;

	if (config.debug.show_developer_console) {
		quick_print(
			renderer.triangles, 
			multiply_alpha(augs::gui::text::format_recent_global_log(assets::font_id::GUI_FONT), 150.f / 255), 
			vec2i(screen_size.x - 300, 0), 
			300
		);

		const auto coords = character_chased_by_camera.alive() ? character_chased_by_camera.get_logic_transform().pos : vec2();
		const auto rot = character_chased_by_camera.alive() ? character_chased_by_camera.get_logic_transform().rotation : 0.f;
		const auto vel = character_chased_by_camera.alive() ? character_chased_by_camera.get<components::rigid_body>().velocity() : vec2();

		const auto gui_style = style(
			assets::font_id::GUI_FONT,
			rgba(255, 255, 255, 150)
		);

		const auto revision_info =
			augs::gui::text::format_as_bbcode(
				typesafe_sprintf(
					"Revision no.: %x %x\nDate: %x\nMessage:\n%x\n",
					HYPERSOMNIA_COMMIT_NUMBER ? std::to_string(HYPERSOMNIA_COMMIT_NUMBER) : "Unknown",
					HYPERSOMNIA_COMMIT_NUMBER ? HYPERSOMNIA_WORKING_TREE_CHANGES.empty() ? "(clean)" : "(dirty)" : "", 
					HYPERSOMNIA_COMMIT_DATE,
					HYPERSOMNIA_COMMIT_MESSAGE.size() < 30 ? HYPERSOMNIA_COMMIT_MESSAGE : HYPERSOMNIA_COMMIT_MESSAGE.substr(0, 30) + "(...)"
				),
				gui_style
			)
		;

		const auto lt_text_formatted = revision_info + format(
			typesafe_sprintf(
				L"Entities: %x\nX: %f2\nY: %f2\nRot: %f2\nVelX: %x\nVelY: %x\n",
				cosmos.get_entities_count(),
				coords.x,
				coords.y,
				rot,
				vel.x,
				vel.y
			) + get_profile_summary() + cosmos.profiler.summary() + L"\n",

			gui_style
		);

		draw_text_at_left_top(renderer, lt_text_formatted + custom_log);
	}
		
	renderer.call_triangles();
	renderer.clear_triangles();

	profiler.triangles.measure(static_cast<std::size_t>(renderer.triangles_drawn_total));
	renderer.triangles_drawn_total = 0;
}

void viewing_session::draw_color_overlay(augs::vertex_triangle_buffer& output, const rgba col) const {
	const auto& camera = audiovisuals.camera.smoothed_camera;
	
	components::sprite overlay;
	overlay.set(assets::game_image_id::BLANK, camera.visible_world_area, col);

	components::sprite::drawing_input in(output);
	in.camera = camera;
	in.renderable_transform = camera.transform;

	overlay.draw(in);
}

void viewing_session::get_visible_entities(
	visible_entities& into,
	const cosmos& cosm
) {
	into.from_camera(audiovisuals.camera.smoothed_camera, cosm);
}