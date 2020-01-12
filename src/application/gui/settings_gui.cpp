#include <cstring>
#include <thread>

#include "augs/log.h"
#include "augs/log_path_getters.h"
#include "augs/window_framework/shell.h"
#include "augs/filesystem/file.h"
#include "augs/templates/introspect.h"
#include "augs/graphics/renderer.h"
#include "augs/templates/enum_introspect.h"
#include "augs/string/format_enum.h"
#include "augs/misc/enum/enum_array.h"
#include "augs/filesystem/directory.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_enum_combo.h"
#include "augs/misc/imgui/imgui_enum_radio.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/audio/audio_context.h"

#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/window.h"

#include "view/necessary_resources.h"

#include "application/config_lua_table.h"
#include "application/gui/settings_gui.h"
#include "augs/network/network_types.h"
#include "augs/audio/sound_sizes.h"
#include "application/gui/config_nvp.h"
#include "application/gui/do_server_vars.h"
#include "application/gui/pretty_tabs.h"

void configuration_subscribers::sync_back_into(config_lua_table& into) const {
	window.sync_back_into(into.window);
}

void configuration_subscribers::apply(const config_lua_table& new_config) const {
	DEBUG_DRAWING = new_config.debug_drawing;
	
	audio_context.apply(new_config.audio);
}

void configuration_subscribers::apply_main_thread(const augs::window_settings& settings) const {
	window.apply(settings);

	const auto screen_size = window.get_screen_size();
	fbos.apply(screen_size);
}

bool settings_gui_state::should_hijack_key() const {
	return hijacking.for_idx != std::nullopt;
}

void settings_gui_state::set_hijacked_key(const augs::event::keys::key k) {
	hijacking.captured = k;
}

int performance_settings::get_default_num_pool_workers() {
	const auto concurrency = static_cast<int>(std::thread::hardware_concurrency());

	const auto audio_threads = 1;
	const auto openal_threads = 1;
	const auto rendering_threads = 1;
	const auto main_threads = 1;

	const auto total_other_threads = 
		audio_threads
		+ rendering_threads
		+ main_threads
		+ openal_threads
	;

	return std::max(0, concurrency - total_other_threads);
}

int performance_settings::get_num_pool_workers() const {
	if (custom_num_pool_workers.is_enabled) {
		return std::max(0, custom_num_pool_workers.value);
	}

	return get_default_num_pool_workers();
}

void settings_gui_state::perform(
	sol::state& lua,
	const augs::audio_context& audio,
	const augs::path_type& config_path_for_saving,
	const config_lua_table& canon_config,
	config_lua_table& config,
	config_lua_table& last_saved_config,
	vec2i screen_size
) {
	auto for_each_input_map = [&](auto callback) {
		callback(config.app_controls);
		callback(config.game_controls);
		callback(config.general_gui_controls);
		callback(config.inventory_gui_controls);
	};

	if (already_bound_popup) {
		const auto buttons = std::vector<editor_popup::button> {
			{
				"Reassign",
				rgba(200, 80, 0, 255),
				rgba(25, 20, 0, 255),
			},

			{
				"Cancel",
				rgba::zero,
				rgba::zero,
			}
		};

		if (const auto result = already_bound_popup->perform(buttons)) {
			if (result == 1) {
				for_each_input_map([&](auto& m) {
					m.erase(*reassignment_request.captured);
				});

				hijacking = reassignment_request;
				reassignment_request = {};
			}

			already_bound_popup = std::nullopt;
		}
	}

	using namespace augs::imgui;

	centered_size_mult = vec2::square(0.65f);
	
	auto settings = make_scoped_window();

	if (!settings) {
		return;
	}

	{
		auto child = scoped_child("settings view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		int field_id = 0;

		do_pretty_tabs(active_pane);

		auto revert = make_revert_button_lambda(config, last_saved_config);

		auto revertable_checkbox = [&](auto l, auto& f, auto&&... args) {
			checkbox(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_slider = [&](auto l, auto& f, auto&&... args) {
			slider(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_drag_rect_bounded_vec2i = [&](auto l, auto& f, auto&&... args) {
			drag_rect_bounded_vec2i(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_drag = [&](auto l, auto& f, auto&&... args) {
			drag(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_drag_vec2 = [&](auto l, auto& f, auto&&... args) {
			drag_vec2(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_color_edit = [&](auto l, auto& f, auto&&... args) {
			color_edit(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_enum = [&](auto l, auto& f, auto&&... args) {
			enum_combo(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto revertable_enum_radio = [&](auto l, auto& f, auto&&... args) {
			std::string ss = l;
			cut_trailing(ss, "#0123456789");
			text(ss);

			auto scope = scoped_indent();

			enum_radio(f, std::forward<decltype(args)>(args)...);
			//revert(f);
		};

		auto revertable_input_text = [&](auto l, auto& f, auto&&... args) {
			input_text(l, f, std::forward<decltype(args)>(args)...);
			revert(f);
		};

		auto do_lag_simulator = [&](auto& sim) {
			//#if !IS_PRODUCTION_BUILD
			revertable_checkbox("Enable lag simulator", sim.is_enabled);

			if (sim.is_enabled) {
				auto& scope_cfg = sim.value;
				auto indent = scoped_indent();

				revertable_slider(SCOPE_CFG_NVP(latency_ms), 0.f, 200.f);
				revertable_slider(SCOPE_CFG_NVP(jitter_ms), 0.f, 100.f);
				revertable_slider(SCOPE_CFG_NVP(loss_percent), 0.f, 100.f);
				revertable_slider(SCOPE_CFG_NVP(duplicates_percent), 0.f, 99.f);
			}

			//#else
			// text_disabled("The network simulation is always disabled in production builds.");
			(void)sim;
			//#endif
		};

		switch (active_pane) {
			case settings_pane::GENERAL: {
				revertable_enum("Launch on game's startup", config.launch_mode);

				revertable_checkbox("Fullscreen", config.window.fullscreen);
				if (!config.window.fullscreen) {
					auto indent = scoped_indent();
					
					{
						revertable_drag_rect_bounded_vec2i("Window position", config.window.position, 0.3f, vec2i{ 0, 0 }, vec2i(10000, 10000));
						revertable_drag_rect_bounded_vec2i("Windowed size", config.window.size, 0.3f, vec2i{ 0, 0 }, vec2i(10000, 10000));
					}

					revertable_checkbox(CONFIG_NVP(window.border));

					text("GUI cursor input source");

					int e = config.window.raw_mouse_input ? 1 : 0;
					ImGui::RadioButton("Raw (traps cursor inside the window)", &e, 1);
					
					if (ImGui::IsItemHovered()) {
						text_tooltip("Game draws its own cursor.\nWhen in GUI mode,\nforces the cursor inside the window.");
					}
#if TODO
					if (config.window.raw_mouse_input) {
						auto indent = scoped_indent();

						revertable_checkbox("But use system cursor for GUI", config.session.use_system_cursor_for_gui);
					}
#endif
					ImGui::RadioButton("System cursor", &e, 0);

					if (ImGui::IsItemHovered()) {
						text_tooltip("When in GUI mode,\nlets the cursor go outside the window.");
					}

					config.window.raw_mouse_input = e != 0;
				}

				input_text<100>(CONFIG_NVP(window.name), ImGuiInputTextFlags_EnterReturnsTrue); revert(config.window.name);

				revertable_enum_radio("Vsync mode", config.window.vsync_mode);
				revertable_checkbox("Hide this window in-game", config.session.hide_settings_ingame);

				{
					auto& scope_cfg = config.http_client;

					revertable_checkbox("Automatically update when the game starts", scope_cfg.update_on_launch);
					revertable_slider(SCOPE_CFG_NVP(update_connection_timeout_secs), 1, 3600);

					input_text<100>(SCOPE_CFG_NVP(application_update_host), ImGuiInputTextFlags_EnterReturnsTrue); revert(config.http_client.application_update_host);
					input_text<100>(SCOPE_CFG_NVP(application_update_path), ImGuiInputTextFlags_EnterReturnsTrue); revert(config.http_client.application_update_path);

					{
						auto& scope_cfg = config;

						input_text<100>(SCOPE_CFG_NVP(server_list_provider), ImGuiInputTextFlags_EnterReturnsTrue); revert(config.server_list_provider);
						input_text("NAT punch provider", config.nat_punch_provider.address, ImGuiInputTextFlags_EnterReturnsTrue); revert(config.nat_punch_provider);
					}
				}

				text_disabled("\n\n");

				ImGui::Separator();

				if (ImGui::Button("Reset all settings to factory defaults")) {
					config = config_lua_table(lua, augs::path_type("default_config.lua"));
				}

				break;
			}
			case settings_pane::RENDERING: {
				enum_combo("Default filtering", config.renderer.default_filtering);

				{
					auto scope = scoped_indent();

					text_disabled(
						"The nearest neighbor filtering will give you a nostalgic, pixel-art feeling.\n"
						"The linear filtering is a little easier on the eyes.\n"
						"Linear filtering is enabled automatically when the camera zooms out."
					);
				}

				revertable_checkbox("Interpolate frames", config.interpolation.enabled);
				
				if (config.interpolation.enabled) {
					auto scope = scoped_indent();

					revertable_slider("Speed", config.interpolation.speed, 50.f, 1000.f);
				}

				revertable_checkbox("Highlight hovered world items", config.drawing.draw_aabb_highlighter);

				if (auto node = scoped_tree_node("Arena mode GUI")) {
					if (auto node = scoped_tree_node("Scoreboard")) {
						auto scope = scoped_indent();
						auto& scope_cfg = config.arena_mode_gui.scoreboard_settings;

						revertable_slider(SCOPE_CFG_NVP(cell_bg_alpha), 0.f, 1.f);
						
						revertable_drag_vec2(SCOPE_CFG_NVP(player_row_inner_padding), 1.f, 0, 20);

						revertable_color_edit(SCOPE_CFG_NVP(background_color));
						revertable_color_edit(SCOPE_CFG_NVP(border_color));

						revertable_slider(SCOPE_CFG_NVP(bg_lumi_mult), 0.f, 5.f);
						revertable_slider(SCOPE_CFG_NVP(text_lumi_mult), 0.f, 5.f);
						revertable_slider(SCOPE_CFG_NVP(current_player_bg_lumi_mult), 0.f, 5.f);
						revertable_slider(SCOPE_CFG_NVP(current_player_text_lumi_mult), 0.f, 5.f);
						revertable_slider(SCOPE_CFG_NVP(dead_player_bg_lumi_mult), 0.f, 1.f);
						revertable_slider(SCOPE_CFG_NVP(dead_player_bg_alpha_mult), 0.f, 1.f);
						revertable_slider(SCOPE_CFG_NVP(dead_player_text_alpha_mult), 0.f, 1.f);
						revertable_slider(SCOPE_CFG_NVP(dead_player_text_lumi_mult), 0.f, 1.f);

						revertable_slider(SCOPE_CFG_NVP(text_stroke_lumi_mult), 0.f, 1.f);
						revertable_slider(SCOPE_CFG_NVP(faction_logo_alpha), 0.f, 1.f);
						revertable_checkbox(SCOPE_CFG_NVP(dark_color_overlay_under_score));
					}

					if (auto node = scoped_tree_node("Buy menu")) {
						auto scope = scoped_indent();
						auto& scope_cfg = config.arena_mode_gui.buy_menu_settings;

						revertable_color_edit(SCOPE_CFG_NVP(disabled_bg));
						revertable_color_edit(SCOPE_CFG_NVP(disabled_active_bg));

						revertable_color_edit(SCOPE_CFG_NVP(already_owns_bg));
						revertable_color_edit(SCOPE_CFG_NVP(already_owns_active_bg));

						revertable_color_edit(SCOPE_CFG_NVP(already_owns_other_type_bg));
						revertable_color_edit(SCOPE_CFG_NVP(already_owns_other_type_active_bg));
					}

					auto& scope_cfg = config.arena_mode_gui;

					if (auto node = scoped_tree_node("Knockouts indicators")) {
						auto scope = scoped_indent();

						revertable_slider(SCOPE_CFG_NVP(between_knockout_boxes_pad), 0u, 20u);
						revertable_slider(SCOPE_CFG_NVP(inside_knockout_box_pad), 0u, 20u);
						revertable_slider(SCOPE_CFG_NVP(weapon_icon_horizontal_pad), 0u, 20u);
						revertable_slider(SCOPE_CFG_NVP(show_recent_knockouts_num), 0u, 20u);
						revertable_slider(SCOPE_CFG_NVP(keep_recent_knockouts_for_seconds), 0.f, 20.f);
						revertable_slider("Max weapon icon height (0 for no limit)", scope_cfg.max_weapon_icon_height, 0u, 100u);
					}

					if (auto node = scoped_tree_node("Money indicator")) {
						auto scope = scoped_indent();

						revertable_drag_rect_bounded_vec2i("Money indicator position", scope_cfg.money_indicator_pos, 0.3f, -vec2i(screen_size), vec2i(screen_size));
						revertable_color_edit("Money indicator color", scope_cfg.money_indicator_color);
						revertable_color_edit("Award indicator color", scope_cfg.award_indicator_color);
						revertable_slider(SCOPE_CFG_NVP(show_recent_awards_num), 0u, 20u);
						revertable_slider(SCOPE_CFG_NVP(keep_recent_awards_for_seconds), 0.f, 20.f);
					}
				}

				if (auto node = scoped_tree_node("Faction view")) {
					auto nha = [&](auto& f) {
						auto& scope_cfg = f;

						revertable_color_edit(SCOPE_CFG_NVP(normal));
						revertable_color_edit(SCOPE_CFG_NVP(hovered));
						revertable_color_edit(SCOPE_CFG_NVP(active));
					};

					auto for_faction = [&](const faction_type f) {
						auto& scope_cfg = config.faction_view.colors[f];

						revertable_color_edit(SCOPE_CFG_NVP(standard));
						revertable_color_edit(SCOPE_CFG_NVP(current_player_text));
						revertable_color_edit(SCOPE_CFG_NVP(background_dark));

						text("Background");

						{
							auto scope = scoped_indent();
							nha(scope_cfg.background);
						}
					};

					augs::for_each_enum_except_bounds([&](const faction_type f) {
						text(format_enum(f));
						auto scope = scoped_indent();
						for_faction(f);
					});
				}

				break;
			}
			case settings_pane::AUDIO: {
				text_color("Volume settings", yellow);

				ImGui::Separator();

				{
					auto& scope_cfg = config.audio_volume;

					revertable_slider(SCOPE_CFG_NVP(master), 0.f, 1.f);
					revertable_slider(SCOPE_CFG_NVP(sound_effects), 0.f, 1.f);
					revertable_slider(SCOPE_CFG_NVP(music), 0.f, 1.f);
				}

				text_disabled("\n\n");

				ImGui::Separator();

				text_color("Sound effect spatialization", yellow);

				ImGui::Separator();

				revertable_checkbox("Enable HRTF", config.audio.enable_hrtf);

				{
					auto scope = scoped_indent(); 

					const auto stat = audio.get_device().get_hrtf_status();
					text(" Status on device:");
					ImGui::SameLine();

					const auto col = stat.success ? green : red;
					text_color(stat.message, col);
				}

				text_disabled("If you experience a drop in sound quality with HRTF,\ntry setting the sample rate of your audio device to 44.1 kHz,\nor consider replacing the hrtf presets found in content/hrtf with your own.");

				{

					auto& scope_cfg = config.sound;

					revertable_enum_radio("Keep listener position at:", scope_cfg.listener_reference);
					revertable_checkbox("Make listener face the same direction as character", scope_cfg.set_listener_orientation_to_character_orientation);
				}

				text_disabled("\n\n");

				ImGui::Separator();
				text_color("Miscellaneous", yellow);
				ImGui::Separator();

				revertable_slider("Speed of sound (m/s)", config.audio.sound_meters_per_second, 50.f, 400.f);
				revertable_slider("Max object speed for doppler calculation", config.sound.max_speed_for_doppler_calculation, 0.f, 10000.f);


				break;
			}
			case settings_pane::CONTROLS: {
				auto& scope_cfg = config.input;

				{
					auto& scope_cfg = config.input.character;
					revertable_drag_vec2(SCOPE_CFG_NVP(crosshair_sensitivity));
					revertable_checkbox(SCOPE_CFG_NVP(keep_movement_forces_relative_to_crosshair));
				}

				revertable_checkbox(SCOPE_CFG_NVP(swap_mouse_buttons_in_akimbo));

				{
					auto& scope_cfg = config.input.game_gui;
					revertable_checkbox(SCOPE_CFG_NVP(allow_switching_to_bare_hands_as_previous_wielded_weapon));
				}

				const auto binding_text_color = rgba(255, 255, 255, 255);
				const auto bindings_title_color = rgba(255, 255, 0, 255);

				int control_i = 0;
				int map_i = 0;

				auto do_bindings_map = [&](const auto& preffix, auto& m) {
					auto id = scoped_id(map_i++);
					using K = typename remove_cref<decltype(m)>::key_type;
					using A = typename remove_cref<decltype(m)>::mapped_type;

					thread_local std::unordered_map<A, std::vector<K>> action_to_keys;
					action_to_keys.clear();

					for (const auto& ka : m) {
						action_to_keys[ka.second].push_back(ka.first);
					}

					ImGui::Separator();
					text_color(preffix, bindings_title_color);
					ImGui::NextColumn();
					ImGui::NextColumn();
					ImGui::NextColumn();
					ImGui::Separator();
					(void)m;
					
					augs::for_each_enum_except_bounds([&](const A a) {
						auto id = scoped_id(static_cast<int>(a));

						auto find_already_assigned_action_in = [&](const auto& mm, const auto key) -> std::optional<std::string> {
							if (const auto found = mapped_or_nullptr(mm, key)) {
								return format_enum(*found);
							}

							return std::nullopt;
						};

						auto find_already_assigned_action = [&](const auto key) -> std::optional<std::string> {
							if (const auto found = find_already_assigned_action_in(config.app_controls, key)) {
								return *found;
							}

							if (const auto found = find_already_assigned_action_in(config.game_controls, key)) {
								return *found;
							}

							if (const auto found = find_already_assigned_action_in(config.general_gui_controls, key)) {
								return *found;
							}

							if (const auto found = find_already_assigned_action_in(config.inventory_gui_controls, key)) {
								return *found;
							}

							return std::nullopt;
						};

						const bool capturing_this_action = hijacking.for_idx == control_i;
						const auto label = format_enum(a);

						{
							auto scope = scoped_style_color(ImGuiCol_Text, binding_text_color);
							text(label);
						}

						ImGui::NextColumn();

						auto& bindings = action_to_keys[a];

						const auto first_key = [&]() -> std::optional<K> {
							if (bindings.size() > 0) {
								return bindings[0];
							}

							return std::nullopt;
						}();

						const auto second_key = [&]() -> std::optional<K> {
							if (bindings.size() > 1) {
								return bindings[1];
							}

							return std::nullopt;
						}();

						auto do_button = [&](const auto key, bool is_secondary) {
							const bool captured = capturing_this_action && is_secondary == hijacking.for_secondary;

							const auto label = [&]() -> std::string {
								if (captured) {
									return "Press a key, ESC to clear, Enter to abort...";
								}

								return key ? key_to_string_shortened(*key) : "(Unassigned)";
							}();

							const auto captured_bg_col = rgba(255, 20, 0, 80);

							const auto text_color = [&]() {
								if (captured) {
									return rgba(255, 40, 0, 255);
								}

								if (key) {
									return white;
								}

								return rgba(255, 255, 255, 120);
							}();

							const auto colors = std::make_tuple(
								cond_scoped_style_color(captured, ImGuiCol_Header, captured_bg_col),
								cond_scoped_style_color(captured, ImGuiCol_HeaderHovered, captured_bg_col),
								cond_scoped_style_color(captured, ImGuiCol_HeaderActive, captured_bg_col),
								scoped_style_color(ImGuiCol_Text, text_color)
							);

							if (ImGui::Selectable(label.c_str(), captured)) {
								if (!should_hijack_key()) {
									hijacking.for_idx = control_i;
									hijacking.for_secondary = is_secondary;
								}
							}
						};

						do_button(first_key, false);
						ImGui::NextColumn();
						do_button(second_key, true);
						ImGui::NextColumn();

						const bool captured_successfully = hijacking.for_idx && hijacking.captured;

						if (capturing_this_action && captured_successfully) {
							const auto new_key = *hijacking.captured;

							if (new_key != augs::event::keys::key::ENTER) {
								const bool should_clear = new_key == augs::event::keys::key::ESC;
								const auto found = find_already_assigned_action(new_key);

								auto make_popup = [&]() {
									const auto key_name = key_to_string_shortened(new_key);
									const auto current_action_str = format_enum(a);

									const auto description = typesafe_sprintf(
										"\"%x\" is already assigned to \"%x\"!\n\nPress \"Reassign\" to clear \"%x\" from \"%x\"\nand assign \"%x\" to \"%x\".\n\n",
										key_name, *found, key_name, *found, key_name, current_action_str
									);

									reassignment_request = hijacking;

									already_bound_popup = editor_popup {
										"Failed",
										description,
										""
									};
								};

								if (hijacking.for_secondary) {
									if (!should_clear && found) {
										make_popup();
									}
									else {
										if (second_key) {
											m.erase(*second_key);
										}

										if (!should_clear) {
											m[new_key] = a;
										}
									}
								}
								else {
									if (!should_clear && found) {
										make_popup();
									}
									else {
										if (first_key) {
											m.erase(*first_key);
										}

										if (!should_clear) {
											m[new_key] = a;
										}
									}
								}
							}

							hijacking = {};
						}

						++control_i;
					});
				};

				ImGui::Columns(3);

				text_disabled("Action");
				ImGui::NextColumn();

				text_disabled("Key");
				ImGui::NextColumn();

				text_disabled("Alternative key");
				ImGui::NextColumn();

				do_bindings_map("Top-level application bindings", config.app_controls);
				text_disabled("\n\n");
				do_bindings_map("Gameplay bindings", config.game_controls);
				text_disabled("\n\n");
				do_bindings_map("General GUI bindings", config.general_gui_controls);
				text_disabled("\n\n");
				do_bindings_map("Inventory GUI bindings", config.inventory_gui_controls);
				text_disabled("\n\n");

				ImGui::Columns(1);

				ImGui::Separator();

				text_disabled("\n");

				if (ImGui::Button("Restore all keybindings from factory defaults")) {
					config.app_controls = canon_config.app_controls;
					config.game_controls = canon_config.game_controls;
					config.general_gui_controls = canon_config.general_gui_controls;
					config.inventory_gui_controls = canon_config.inventory_gui_controls;
				}


				break;
			}
			case settings_pane::GAMEPLAY: {
				{
					auto& scope_cfg = config.arena_mode_gui;
					revertable_checkbox("Show contextual tips", scope_cfg.context_tip_settings.is_enabled);

					if (scope_cfg.context_tip_settings.is_enabled) {
						auto& scope_cfg = config.arena_mode_gui.context_tip_settings.value;
						auto indent = scoped_indent();

						revertable_slider(SCOPE_CFG_NVP(tip_offset_mult), 0.f, 1.f);

						revertable_color_edit(SCOPE_CFG_NVP(tip_text_color));
						revertable_color_edit(SCOPE_CFG_NVP(bound_key_color));
					}
				}

				if (auto node = scoped_tree_node("Camera")) {
					auto& scope_cfg = config.camera;

					revertable_slider(SCOPE_CFG_NVP(look_bound_expand), 0.2f, 2.f);
					
					revertable_checkbox(SCOPE_CFG_NVP(enable_smoothing));

					if (scope_cfg.enable_smoothing) {
						auto indent = scoped_indent();

						revertable_slider(SCOPE_CFG_NVP(smoothing.averages_per_sec), 0.f, 100.f); 
						revertable_slider(SCOPE_CFG_NVP(smoothing.average_factor), 0.01f, 0.95f); 
					}

					revertable_checkbox(SCOPE_CFG_NVP(adjust_zoom_to_available_fog_of_war_size));
				}

				{
					auto& scope_cfg = config.drawing;

					if (auto node = scoped_tree_node("In-world HUD")) {
						revertable_checkbox("Draw Health bar and ammo bar", config.drawing.draw_hp_bar);
						revertable_checkbox("Draw Personal Electricity bar", config.drawing.draw_pe_bar);
						revertable_checkbox("Draw Consciousness bar", config.drawing.draw_cp_bar);

						revertable_checkbox(SCOPE_CFG_NVP(draw_weapon_laser));
						revertable_checkbox(SCOPE_CFG_NVP(draw_crosshairs));
						revertable_checkbox(SCOPE_CFG_NVP(draw_nicknames));
						revertable_checkbox(SCOPE_CFG_NVP(draw_health_numbers));

						revertable_checkbox("Draw teammate indicators", config.drawing.draw_teammate_indicators.is_enabled);

						if (config.drawing.draw_teammate_indicators.is_enabled) {
							auto indent = scoped_indent();
							revertable_slider("Alpha##colors", config.drawing.draw_teammate_indicators.value, 0.f, 1.f);
						}

						revertable_checkbox("Draw tactical indicators", config.drawing.draw_teammate_indicators.is_enabled);
						text_disabled("(for example, the dropped bomb's location)");

						if (config.drawing.draw_tactical_indicators.is_enabled) {
							auto indent = scoped_indent();
							revertable_slider("Alpha##colors2", config.drawing.draw_tactical_indicators.value, 0.f, 1.f);
						}

						revertable_checkbox("Draw danger indicators", config.drawing.draw_danger_indicators.is_enabled);

						if (config.drawing.draw_danger_indicators.is_enabled) {
							auto indent = scoped_indent();
							revertable_color_edit("Color", config.drawing.draw_danger_indicators.value);
						}

						revertable_checkbox(SCOPE_CFG_NVP(draw_offscreen_indicators));

						if (scope_cfg.draw_offscreen_indicators) {
							auto indent = scoped_indent();

							revertable_enum_radio(SCOPE_CFG_NVP(offscreen_reference_mode));
							revertable_checkbox(SCOPE_CFG_NVP(draw_offscreen_callouts));

							revertable_slider(SCOPE_CFG_NVP(nickname_characters_for_offscreen_indicators), 0, static_cast<int>(max_nickname_length_v));

							revertable_slider(SCOPE_CFG_NVP(show_danger_indicator_for_seconds), 0.f, 20.f);
							revertable_slider(SCOPE_CFG_NVP(fade_danger_indicator_for_seconds), 0.f, 20.f);

							revertable_slider(SCOPE_CFG_NVP(show_death_indicator_for_seconds), 0.f, 20.f);
							revertable_slider(SCOPE_CFG_NVP(fade_death_indicator_for_seconds), 0.f, 20.f);
						}

						revertable_checkbox("Draw callout indicators", config.drawing.draw_callout_indicators.is_enabled);

						if (config.drawing.draw_callout_indicators.is_enabled) {
							auto indent = scoped_indent();
							revertable_slider("Alpha##callouts", config.drawing.draw_callout_indicators.value, 0.f, 1.f);
						}

						revertable_checkbox("Draw area markers", config.drawing.draw_area_markers.is_enabled);

						if (config.drawing.draw_area_markers.is_enabled) {
							auto indent = scoped_indent();
							revertable_slider("Alpha##markers", config.drawing.draw_area_markers.value, 0.f, 1.f);
						}
					}

					if (auto node = scoped_tree_node("Static HUD")) {
						revertable_checkbox(SCOPE_CFG_NVP(print_current_character_callout));

						{
							revertable_drag_vec2(SCOPE_CFG_NVP(radar_pos));
						}

					}
				}

				if (auto node = scoped_tree_node("Inventory behaviour")) {
					auto& scope_cfg = config.game_gui;

					revertable_checkbox(SCOPE_CFG_NVP(autodrop_magazines_of_dropped_weapons));
					revertable_checkbox(SCOPE_CFG_NVP(autodrop_holstered_armed_explosives));
					revertable_checkbox(SCOPE_CFG_NVP(autocollapse_hotbar_buttons));
				}

				if (auto node = scoped_tree_node("Fog of war")) {
					auto& scope_cfg = config.drawing.fog_of_war_appearance;

					revertable_color_edit("Field of view overlay color", scope_cfg.overlay_color);

					text("Overlay color on:");

					{
						auto indent = scoped_indent();

						auto& f = scope_cfg.overlay_color_on_visible;
						if (ImGui::RadioButton("Visible area", f)) {
							f = true;
						}

						if (ImGui::RadioButton("The occlusion fog", !f)) {
							f = false;
						}
						text("\n");
						revert(f);
					}
				}

				// revertable_checkbox("Draw gameplay GUI", config.drawing.draw_character_gui); revert(config.drawing.draw_character_gui);
				break;
			}
			case settings_pane::CLIENT: {
				auto& scope_cfg = config.client;

				const auto label = typesafe_sprintf("Chosen nickname (%x-%x characters)", min_nickname_length_v, max_nickname_length_v);

				revertable_input_text(label, scope_cfg.nickname);
				revertable_input_text(SCOPE_CFG_NVP(rcon_password));

				revertable_checkbox("Record demo", scope_cfg.demo_recording_path.is_enabled);

				if (scope_cfg.demo_recording_path.is_enabled) {
					auto scope = scoped_indent();
					
					input_text<512>("Target demo directory", scope_cfg.demo_recording_path.value, ImGuiInputTextFlags_EnterReturnsTrue); revert(scope_cfg.demo_recording_path.value);
					revertable_slider(SCOPE_CFG_NVP(flush_demo_to_disk_once_every_secs), 1u, 120u);
				}

				{
					auto& scope_cfg = config.arena_mode_gui;
					revertable_checkbox(SCOPE_CFG_NVP(show_client_resyncing_notifier));
				}

				if (auto node = scoped_tree_node("Chat window")) {
					auto& scope_cfg = config.client.client_chat;

					revertable_slider(SCOPE_CFG_NVP(chat_window_width), 100u, 500u);
					revertable_drag_rect_bounded_vec2i(SCOPE_CFG_NVP(chat_window_offset), 0.3f, -vec2i(screen_size), vec2i(screen_size));

					revertable_slider(SCOPE_CFG_NVP(show_recent_chat_messages_num), 0u, 30u);
					revertable_slider(SCOPE_CFG_NVP(keep_recent_chat_messages_for_seconds), 0.f, 30.f);
				}

				if (auto node = scoped_tree_node("Advanced")) {
					revertable_enum_radio("Spectate:", scope_cfg.spectated_arena_type, true);
					revertable_slider(SCOPE_CFG_NVP(max_buffered_server_commands), 0u, 10000u);
					revertable_slider(SCOPE_CFG_NVP(max_predicted_client_commands), 0u, 3000u);
				}

				ImGui::Separator();

				text_color("Lag compensation", yellow);

				ImGui::Separator();

				do_lag_simulator(config.client.network_simulator);

				{
					{
						auto& scope_cfg = config.simulation_receiver;
						revertable_slider(SCOPE_CFG_NVP(misprediction_smoothing_multiplier), 0.f, 3.f);
					}

					{
						auto& scope_cfg = config.lag_compensation;
						revertable_checkbox(SCOPE_CFG_NVP(confirm_controlled_character_death));

						augs::introspect(
							[&](const std::string& label, auto& field){
								revertable_checkbox(format_field_name(label), field);
							},
							scope_cfg.effect_prediction
						); 
					}
				}

				ImGui::Separator();

				text_color("Jitter compensation", yellow);

				ImGui::Separator();

				{
					auto& scope_cfg = config.client.net.jitter;

					revertable_slider(SCOPE_CFG_NVP(buffer_at_least_steps), 0u, 10u);
					revertable_slider(SCOPE_CFG_NVP(buffer_at_least_ms), 0u, 100u);
					revertable_slider(SCOPE_CFG_NVP(max_commands_to_squash_at_once), uint8_t(0), uint8_t(255));
				}

				break;
			}
			case settings_pane::SERVER: {
				ImGui::Separator();

				text_color("Arenas", yellow);

				ImGui::Separator();

				do_server_vars(
					config.server_solvable,
					last_saved_config.server_solvable
				);

				ImGui::Separator();

				do_server_vars(
					config.server,
					last_saved_config.server
				);

				if (auto node = scoped_tree_node("RCON")) {
					auto& scope_cfg = config.private_server;

					input_text(SCOPE_CFG_NVP(rcon_password)); revert(scope_cfg.rcon_password);
					text_disabled("A rcon can change maps, alter modes, kick/ban players and perform other administrative activities.");

					input_text(SCOPE_CFG_NVP(master_rcon_password)); revert(scope_cfg.master_rcon_password);
					text_disabled("A master rcon can additionally change the rcon password in case of an emergency.");

					{
						auto& scope_cfg = config.server;
						revertable_checkbox(SCOPE_CFG_NVP(auto_authorize_loopback_for_rcon));
					}
				}

				break;
			}
			case settings_pane::EDITOR: {
				if (auto node = scoped_tree_node("General")) {
					revertable_checkbox("Enable autosave", config.editor.autosave.enabled);
	
					if (config.editor.autosave.enabled) {
						auto scope = scoped_indent();
						text("Autosave once per");
						ImGui::SameLine();
						revertable_drag("minutes", config.editor.autosave.once_every_min, 0.002, 0.05, 2000.0);

						revertable_checkbox("Autosave when window loses focus", config.editor.autosave.on_lost_focus);
					}
					
#if TODO
					text("Remember last");
					ImGui::SameLine();

					revertable_drag("commands for undoing", config.editor.remember_last_n_commands, 1, 10, 2000);
#endif

					if (auto node = scoped_tree_node("Test scene generation")) {
						auto& scope_cfg = config.editor.test_scene;
						
						revertable_slider(SCOPE_CFG_NVP(scene_tickrate), 10u, 300u);
						revertable_checkbox(SCOPE_CFG_NVP(start_bomb_mode));
					}
				}	
				
				if (auto node = scoped_tree_node("Interface")) {
					if (auto node = scoped_tree_node("Grid")) {
						auto& scope_cfg = config.editor.grid.render;

						if (auto lines = scoped_tree_node("Line colors")) {
							for (std::size_t i = 0; i < scope_cfg.line_colors.size(); ++i) {
								revertable_color_edit(std::to_string(i), scope_cfg.line_colors[i]);
							}
						}

						{
							auto& po2 = scope_cfg.maximum_power_of_two;

							revertable_slider("Maximum power of 2", po2, 3u, 20u);
							ImGui::SameLine();
							text(typesafe_sprintf("(Max grid size: %x)", 1 << po2));
						}

						revertable_slider("Alpha", scope_cfg.alpha_multiplier, 0.f, 1.f);
						revertable_slider(SCOPE_CFG_NVP(hide_grids_smaller_than), 0u, 128u);
					}

					if (auto node = scoped_tree_node("Camera")) {
						revertable_drag("Panning speed", config.editor.camera.panning_speed, 0.001f, -10.f, 10.f);
					}

					if (auto node = scoped_tree_node("\"Go to\" dialogs")) {
						revertable_slider("Width", config.editor.go_to.dialog_width, 30u, static_cast<unsigned>(screen_size.x));
						revertable_slider("Number of lines to show", config.editor.go_to.num_lines, 1u, 300u);
					}

					if (auto node = scoped_tree_node("Entity selections")) {
						auto& scope_cfg = config.editor;
						revertable_checkbox(SCOPE_CFG_NVP(keep_source_entities_selected_on_mirroring));
					}
				}

				if (auto node = scoped_tree_node("Appearance")) {
					if (auto node = scoped_tree_node("Property editor")) {
						auto& scope_cfg = config.editor.property_editor;

						revertable_color_edit(SCOPE_CFG_NVP(different_values_frame_bg));
						revertable_color_edit(SCOPE_CFG_NVP(different_values_frame_hovered_bg));
						revertable_color_edit(SCOPE_CFG_NVP(different_values_frame_active_bg));
					}

					if (auto node = scoped_tree_node("Entity highlights")) {
						{
							auto& scope_cfg = config.editor.entity_selector;

							revertable_color_edit(SCOPE_CFG_NVP(hovered_color));
							revertable_color_edit(SCOPE_CFG_NVP(hovered_dashed_line_color));

							revertable_color_edit(SCOPE_CFG_NVP(selected_color));
							revertable_color_edit(SCOPE_CFG_NVP(held_color));
						}

						{
							auto& scope_cfg = config.editor;

							revertable_color_edit(SCOPE_CFG_NVP(controlled_entity_color));
							revertable_color_edit(SCOPE_CFG_NVP(matched_entity_color));
						}
					}

					if (auto node = scoped_tree_node("Action indicator")) {
						auto& scope_cfg = config.editor.action_indicator;

						revertable_color_edit(SCOPE_CFG_NVP(bg_color));
						revertable_color_edit(SCOPE_CFG_NVP(bg_border_color));

						revertable_slider(SCOPE_CFG_NVP(max_width), 10u, 1000u);
						revertable_slider(SCOPE_CFG_NVP(show_for_ms), 0u, 20000u);

						revertable_slider(SCOPE_CFG_NVP(show_for_ms), 0u, 20000u);

						revertable_drag_vec2(SCOPE_CFG_NVP(text_padding));
						revertable_drag_vec2(SCOPE_CFG_NVP(offset));
					}

					auto& scope_cfg = config.editor;

					revertable_color_edit(SCOPE_CFG_NVP(tutorial_text_color));
					revertable_color_edit(SCOPE_CFG_NVP(rectangular_selection_color));
					revertable_color_edit(SCOPE_CFG_NVP(rectangular_selection_border_color));
				}

				if (auto node = scoped_tree_node("Player")) {
					auto& scope_cfg = config.editor.player;

					revertable_slider(SCOPE_CFG_NVP(snapshot_interval_in_steps), 400u, 5000u);
				}

				if (auto node = scoped_tree_node("Debug")) {
					auto& scope_cfg = config.editor;

					revertable_checkbox(SCOPE_CFG_NVP(save_entropies_to_live_file));
				}

				break;
			}
			case settings_pane::GUI: {
				if (auto node = scoped_tree_node("GUI font")) {
					auto scope = scoped_indent();

					revertable_slider("Size in pixels", config.gui_fonts.gui.size_in_pixels, 5.f, 64.f);
				}

				text(
					"Note: this is the original ImGui style tweaker.\n"
					"It is used because imgui is being continuously improved,\n"
					"so keeping it up to date by ourselves would be pretty hard.\n\n"
					"To save your changes to the local configuration file,\n"
					"You need to push Save Ref and only then Save settings at the bottom."
				);
				
				ImGui::Separator();
				ImGui::ShowStyleEditor(&config.gui_style);

				break;
			}
			case settings_pane::PERFORMANCE: {
				text_color("Sound quality", yellow);

				ImGui::Separator();

				{
					auto& scope_cfg = config.sound;

					revertable_enum_radio(SCOPE_CFG_NVP(processing_frequency));
					revertable_slider(SCOPE_CFG_NVP(max_simultaneous_bullet_trace_sounds), 0, 20);
					revertable_slider(SCOPE_CFG_NVP(max_short_sounds), 0, static_cast<int>(SOUNDS_SOURCES_IN_POOL));

					revertable_slider(SCOPE_CFG_NVP(missile_impact_sound_cooldown_duration), 1.f, 100.f);
					revertable_slider(SCOPE_CFG_NVP(missile_impact_occurences_before_cooldown), 0, 10);
				}

				ImGui::Separator();

				text_color("General effects", yellow);

				ImGui::Separator();

				{
					auto& scope_cfg = config.performance.special_effects;
					revertable_slider(SCOPE_CFG_NVP(particle_stream_amount), 0.f, 1.f);
					revertable_slider(SCOPE_CFG_NVP(particle_burst_amount), 0.f, 1.f);
				}

				ImGui::Separator();

				text_color("Explosion effects intensity", yellow);

				ImGui::Separator();

				{
					auto& scope_cfg = config.performance.special_effects.explosions;

					revertable_slider(SCOPE_CFG_NVP(sparkle_amount), 0.f, 1.f);
					revertable_slider(SCOPE_CFG_NVP(thunder_amount), 0.f, 1.f);
					revertable_slider(SCOPE_CFG_NVP(smoke_amount), 0.f, 1.f);
				}

				ImGui::Separator();

				text_color("Light", yellow);

				ImGui::Separator();

				{
					auto& scope_cfg = config.drawing;
					revertable_checkbox(SCOPE_CFG_NVP(occlude_neons_under_sentiences));
					revertable_checkbox(SCOPE_CFG_NVP(occlude_neons_under_other_bodies));
				}


				{
					auto& scope_cfg = config.performance;
					revertable_enum_radio(SCOPE_CFG_NVP(wall_light_drawing_precision));
				}

				ImGui::Separator();

				text_color("Decorations", yellow);

				ImGui::Separator();

				auto& scope_cfg = config.lag_compensation;

				{
					revertable_checkbox(SCOPE_CFG_NVP(simulate_decorative_organisms_during_reconciliation));
				}

				ImGui::Separator();

				text_color("Multithreading", yellow);

				ImGui::Separator();

				{
					auto& scope_cfg = config.performance;

					revertable_enum_radio(SCOPE_CFG_NVP(swap_window_buffers_when));

					const auto concurrency = static_cast<int>(std::thread::hardware_concurrency());

					{
						text("Concurrent hardware threads:");
						ImGui::SameLine();
						text_color(typesafe_sprintf("%x", concurrency), green);
					}

					{
						const auto default_n = performance_settings::get_default_num_pool_workers();
						text("Default number of thread pool workers:");
						ImGui::SameLine();
						text_color(typesafe_sprintf("%x\n\n", default_n), default_n == 0 ? red : green);
					}


					{
						auto& cn = scope_cfg.custom_num_pool_workers;
						revertable_checkbox("Custom number of thread pool workers", cn.is_enabled);

						if (cn.is_enabled) {
							auto indent = scoped_indent();
							revertable_slider("##ThreadCount", cn.value, 0, concurrency * 3);
						}
					}

					revertable_slider(SCOPE_CFG_NVP(max_particles_in_single_job), 1000, 20000);
				}

				break;
			}
			case settings_pane::DEBUG: {
				const auto cwd = augs::get_current_working_directory();
				text("Working directory: %x", cwd, std::filesystem::absolute(cwd));
				text("Cache folder location: %x (%x)", GENERATED_FILES_DIR, std::filesystem::absolute(GENERATED_FILES_DIR));

				if (ImGui::Button("Dump debug log")) {
					get_dumped_log_path();
					
					const auto logs = program_log::get_current().get_complete();
					auto failure_log_path = augs::path_type(get_dumped_log_path());

					augs::save_as_text(failure_log_path, logs);

					augs::open_text_editor(failure_log_path.string());
					augs::open_text_editor(failure_log_path.replace_filename("").string());
				}

#if !PLATFORM_WINDOWS
				text(u8"Test: いい товарищ żółćńźś");
#endif
				ImGui::Separator();

				{
					auto& scope_cfg = config.debug;
					revertable_checkbox(SCOPE_CFG_NVP(log_solvable_hashes));
				}

				revertable_checkbox("Show developer console", config.session.show_developer_console);
				revertable_checkbox("Log keystrokes", config.window.log_keystrokes);
				revertable_slider("Camera query aabb mult", config.session.camera_query_aabb_mult, 0.10f, 5.f);
				
				revertable_checkbox("Draw debug lines", config.debug_drawing.enabled);

				if (config.debug_drawing.enabled) {
					auto indent = scoped_indent();

					augs::introspect(
						[&](const std::string& label, auto& field){
							if (label != "enabled") {
								revertable_checkbox(format_field_name(label), field);
							}
						},
						config.debug_drawing
					); 
				}

#if BUILD_TEST_SCENES
				text("Test scenes (%x)", "built-in");
#else
				text("Test scenes (%x)", "not built-in");
#endif

				{
					auto indent = scoped_indent();

					revertable_checkbox("Create minimal", config.test_scene.create_minimal);
					revertable_slider("Tickrate", config.test_scene.scene_tickrate, 10u, 300u);
				}

				{
					auto& scope_cfg = config.debug;
					revertable_checkbox(SCOPE_CFG_NVP(measure_atlas_uploading));
				}

				text("Content regeneration");

				{
					auto indent = scoped_indent();
					auto& scope_cfg = config.content_regeneration;

					revertable_checkbox(SCOPE_CFG_NVP(regenerate_every_time));
					revertable_checkbox(SCOPE_CFG_NVP(rescan_assets_on_window_focus));

					ImGui::SameLine();

					const auto concurrency = std::thread::hardware_concurrency();
					const auto t_max = concurrency * 2;

					text_disabled("(Value of 0 tells regenerators to not spawn any additional workers)");
					text_disabled(typesafe_sprintf("(std::thread::hardware_concurrency() = %x)", concurrency));

					revertable_slider(SCOPE_CFG_NVP(atlas_blitting_threads), 1u, t_max);
					revertable_slider(SCOPE_CFG_NVP(neon_regeneration_threads), 1u, t_max);
				}

				break;
			}
			default: {
				ensure(false && "Unknown settings pane type");
				break;
			}
		}
	}

	{
		auto scope = scoped_child("save revert");

		ImGui::Separator();

		if (config != last_saved_config) {
			if (ImGui::Button("Save settings")) {
				augs::timer save_timer;
				last_saved_config = config;
				config.save_patch(lua, canon_config, config_path_for_saving);
				LOG("Saved new config in: %x ms", save_timer.get<std::chrono::milliseconds>());
			}

			ImGui::SameLine();

			if (ImGui::Button("Revert settings")) {
				config = last_saved_config;
				ImGui::GetStyle() = config.gui_style;
			}
		}
	}
}

#include "application/gui/arena_chooser.h"
#include "application/arena/arena_paths.h"
#include "application/gui/config_nvp.h"

void perform_arena_chooser(
	arena_identifier& current_arena
) {
	thread_local arena_chooser chooser;

	chooser.perform(
		"Arena",
		current_arena,
		augs::path_type(OFFICIAL_ARENAS_DIR),
		augs::path_type(COMMUNITY_ARENAS_DIR),
		[&](const auto& new_choice) {
			current_arena = new_choice.path.string();
		}
	);
}

void do_server_vars(
	server_solvable_vars& vars,
	server_solvable_vars& last_saved_vars
) {
	using namespace augs::imgui;

	int field_id = 499999;

	auto& scope_cfg = vars;

	auto revert = make_revert_button_lambda(vars, last_saved_vars);

	perform_arena_chooser(vars.current_arena);
	revert(vars.current_arena);

	input_text(SCOPE_CFG_NVP(override_default_ruleset)); revert(scope_cfg.override_default_ruleset);
}

void do_server_vars(
	server_vars& vars,
	server_vars& last_saved_vars
) {
	using namespace augs::imgui;

	int field_id = 999999;

	auto& scope_cfg = vars;

	auto revert = make_revert_button_lambda(vars, last_saved_vars);

	auto revertable_slider = [&](auto l, auto& f, auto&&... args) {
		slider(l, f, std::forward<decltype(args)>(args)...);
		revert(f);
	};

	auto revertable_input_text = [&](auto l, auto& f, auto&&... args) {
		input_text(l, f, std::forward<decltype(args)>(args)...);
		revert(f);
	};

	text_color("General", yellow);

	ImGui::Separator();

	revertable_input_text(SCOPE_CFG_NVP(notified_server_list.address));
	revertable_input_text(SCOPE_CFG_NVP(server_name));

	if (auto node = scoped_tree_node("Time limits")) {
		revertable_slider(SCOPE_CFG_NVP(kick_if_no_messages_for_secs), 2u, 300u);
		revertable_slider(SCOPE_CFG_NVP(kick_if_away_from_keyboard_for_secs), 20u, 6000u);
		revertable_slider(SCOPE_CFG_NVP(time_limit_to_enter_game_since_connection), 5u, 300u);
	}

	ImGui::Separator();

	text_color("Dedicated server", yellow);

	ImGui::Separator();

	revertable_slider(SCOPE_CFG_NVP(sleep_mult), 0.0f, 0.9f);
}

#undef CONFIG_NVP
