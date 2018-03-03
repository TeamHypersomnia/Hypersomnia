#include "augs/templates/string_templates.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/addons/imguitabwindow/imguitabwindow.h"
#include "augs/filesystem/directory.h"
#include "augs/templates/thread_templates.h"
#include "augs/templates/chrono_templates.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/shell.h"

#include "game/detail/visible_entities.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_paths.h"

#include <imgui/imgui_internal.h>

#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/lua_file.h"

void editor_setup::open_last_folders(sol::state& lua) {
	catch_popup([&]() { ::open_last_folders(lua, signi); });
	base::refresh();
}

double editor_setup::get_audiovisual_speed() const {
	return player.get_speed();
}

const cosmos& editor_setup::get_viewed_cosmos() const {
	return anything_opened() ? work().world : cosmos::zero; 
}

real32 editor_setup::get_interpolation_ratio() const {
	return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
}

entity_id editor_setup::get_viewed_character_id() const {
	return anything_opened() ? work().local_test_subject : entity_id();
}

const_entity_handle editor_setup::get_viewed_character() const {
	return get_viewed_cosmos()[get_viewed_character_id()];
}

const all_viewables_defs& editor_setup::get_viewable_defs() const {
	return anything_opened() ? work().viewables : all_viewables_defs::empty;
}

void editor_setup::unhover() {
	hovered_entity = {};
}

bool editor_setup::is_editing_mode() const {
	return player.paused;
}

std::optional<camera_cone> editor_setup::get_custom_camera() const {
	auto maybe_panning = [this]() -> std::optional<camera_cone> { 
		if (anything_opened() && is_editing_mode()) {
			if (view().panned_camera) {
				return view().panned_camera;
			}
		}

		return std::nullopt;
	};

	if (anything_opened() && is_editing_mode()) {
		if (const auto match = get_matching_go_to_entity()) {
			camera_cone centered_on_match;

			if (const auto panning = maybe_panning()) {
				/* Propagate zoom from custom panning */
				centered_on_match = *panning;
			}

			if (const auto transform = match.find_logic_transform()) {
				centered_on_match.transform.pos = transform->pos;
			}
			else {
				LOG("WARNING: transform of %x could not be found.", match);
			}

			return centered_on_match;
		}
	}

	return maybe_panning();
}

const_entity_handle editor_setup::get_matching_go_to_entity() const {
	if (show_go_to_entity && matching_go_to_entities.size() > 0) {
		return work().world[matching_go_to_entities[go_to_entities_selected_index]];
	}

	return work().world[entity_id()];
}

void editor_setup::on_folder_changed() {
	hovered_entity = {};
	player.paused = true;
	finish_rectangular_selection();
}

void editor_setup::set_popup(const editor_popup p) {
	ok_only_popup = p;

	const auto logged = typesafe_sprintf(
		"%x\n%x\n%x", 
		p.title, p.message, p.details
	);

	LOG(logged);

	augs::save_as_text(LOG_FILES_DIR "last_editor_message.txt", logged);
}

void editor_setup::set_locally_viewed(const entity_id id) {
	work().local_test_subject = id;
	view().panned_camera = std::nullopt;
}

editor_setup::editor_setup(
	sol::state& lua
) : 
	destructor_input{ lua },
	recent(lua) 
{
	augs::create_directories(get_untitled_dir());

	open_last_folders(lua);
}

editor_setup::editor_setup(
	sol::state& lua, 
	const augs::path_type& intercosm_path
) : 
	destructor_input{ lua },
	recent(lua) 
{
	augs::create_directories(get_untitled_dir());

	open_last_folders(lua);
	open_folder_in_new_tab({ lua, intercosm_path });
}

void editor_setup::force_autosave_now() const {
	autosave.save(destructor_input.lua, signi);
}

editor_setup::~editor_setup() {
	force_autosave_now();
}

void editor_setup::control(
	const cosmic_entropy& entropy
) {
	total_collected_entropy += entropy;
}

void editor_setup::accept_game_gui_events(
	const cosmic_entropy& entropy
) {
	control(entropy);
}

void editor_setup::customize_for_viewing(config_lua_table& config) const {
	if (anything_opened()) {
		config.window.name = "Editor - " + folder().get_display_path();
	}
	else {
		config.window.name = "Editor";
	}

	if (player.paused) {
		config.drawing.draw_aabb_highlighter = false;
		config.interpolation.enabled = false;
	}

	return;
}

void editor_setup::apply(const config_lua_table& cfg) {
	settings = cfg.editor;
	return;
}

void editor_setup::open_folder_in_new_tab(const path_operation op) {
	for (std::size_t i = 0; i < signi.folders.size(); ++i) {
		if (signi.folders[i].current_path == op.path) {
			set_current(static_cast<folder_index>(i));
			return;
		}
	}

	try_to_open_new_folder(
		[this, op](editor_folder& f) {
			f.set_folder_path(op.lua, op.path, recent);
			f.load_folder();
			f.history.mark_current_revision_as_saved();
		}
	);
}

void editor_setup::save_current_folder() {
	folder().save_folder();
	folder().history.mark_current_revision_as_saved();
}

void editor_setup::save_current_folder_to(const path_operation op) {
	folder().set_folder_path(op.lua, op.path, recent);
	save_current_folder();
}

void editor_setup::fill_with_minimal_scene(sol::state& lua) {
#if BUILD_TEST_SCENES
	if (anything_opened()) {
		work().make_test_scene(lua, { true, 144 } );

		clear_all_selections();
	}
#endif
}

void editor_setup::fill_with_test_scene(sol::state& lua) {
#if BUILD_TEST_SCENES
	if (anything_opened()) {
		work().make_test_scene(lua, { false, 144 } );

		clear_all_selections();
	}
#endif
}

void editor_setup::perform_custom_imgui(
	sol::state& lua,
	augs::window& owner,
	const bool in_direct_gameplay,

	const camera_cone current_cone
) {
	using namespace augs::imgui;

	autosave.advance(lua, signi, settings.autosave);

	auto path_op = [&](const auto& path) {
		return path_operation{ lua, path };
	};

	auto item_if_tabs_and = [this](const bool condition, const char* label, const char* shortcut = nullptr) {
		return ImGui::MenuItem(label, shortcut, nullptr, condition && anything_opened());
	};

	auto item_if_tabs = [&](const char* label, const char* shortcut = nullptr) {
		return item_if_tabs_and(true, label, shortcut);
	};

	const auto mouse_pos = vec2i(ImGui::GetIO().MousePos);
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);
	const auto world_cursor_pos = current_cone.to_world_space(screen_size, mouse_pos);

	const auto& g = *ImGui::GetCurrentContext();
	const auto menu_bar_size = ImVec2(g.IO.DisplaySize.x, g.FontBaseSize + g.Style.FramePadding.y * 2.0f);

	if (!in_direct_gameplay) {
		{
			/* We don't want ugly borders in our menu bar */
			auto window_border_size = scoped_style_var(ImGuiStyleVar_WindowBorderSize, 0.0f);
			auto frame_border_size = scoped_style_var(ImGuiStyleVar_FrameBorderSize, 0.0f);

			if (auto main_menu = scoped_main_menu_bar()) {
				if (auto menu = scoped_menu("File")) {
					if (ImGui::MenuItem("New", "CTRL+N")) {
						new_tab();
					}

					if (ImGui::MenuItem("Open", "CTRL+O")) {
						open(owner);
					}

					if (auto menu = scoped_menu("Recent files", !recent.empty())) {
						/*	
							IMPORTANT! recent.paths can be altered in the loop by loading a intercosm,
							thus we need to copy its contents.
						*/

						const auto recent_paths = recent.paths;

						for (const auto& target_path : recent_paths) {
							const auto str = augs::to_display_path(target_path).string();

							if (ImGui::MenuItem(str.c_str())) {
								open_folder_in_new_tab(path_op(target_path));
							}
						}

						ImGui::Separator();

						if (ImGui::MenuItem("Clear Recent Items")) {
							recent.clear(lua);
						}
					}

					ImGui::Separator();

					if (item_if_tabs("Save", "CTRL+S")) {
						save(lua, owner);
					}

					if (item_if_tabs("Save as", "F12")) {
						save_as(owner);
					}

					ImGui::Separator();

					const auto close_str = [&]() -> std::string {
						if (anything_opened()) {
							return std::string("Close ") + folder().get_display_path();
						}

						return "Close";
					}();

					if (item_if_tabs(close_str.c_str(), "CTRL+W")) {
						close_folder();
					}

					if (item_if_tabs("Close all")) {
						while (anything_opened()) {
							close_folder();
						}
					}
				}

				if (auto menu = scoped_menu("Edit")) {
					if (item_if_tabs("Undo", "CTRL+Z")) {}
					if (item_if_tabs("Redo", "CTRL+SHIFT+Z")) {}
					ImGui::Separator();
					if (item_if_tabs("Cut", "CTRL+X")) {}
					if (item_if_tabs("Copy", "CTRL+C")) {}
					if (item_if_tabs("Paste", "CTRL+V")) {}
					ImGui::Separator();

#if BUILD_TEST_SCENES
					if (item_if_tabs("Fill with test scene", "SHIFT+F5")) {
						fill_with_test_scene(lua);
					}

					if (item_if_tabs("Fill with minimal scene", "CTRL+SHIFT+F5")) {
						fill_with_minimal_scene(lua);
					}
#else
					if (item_if_tabs_and(false, "Fill with test scene", "SHIFT+F5")) {}
					if (item_if_tabs_and(false, "Fill with minimal scene", "CTRL+SHIFT+F5")) {}
#endif
				}
				if (auto menu = scoped_menu("View")) {
					if (item_if_tabs("Summary")) {
						show_summary = true;
					}
					if (item_if_tabs("Player")) {
						player.show = true;
					}

					ImGui::Separator();
					ImGui::MenuItem("(State)", NULL, false, false);

					if (item_if_tabs("Common")) {
						show_common_state = true;
					}

					if (item_if_tabs("Entities")) {
						show_entities = true;
					}
				}
			}
		}

		if (anything_opened()) {
			if (const auto tab_menu = scoped_tab_menu_bar(menu_bar_size.y)) {
				{
					using namespace ImGui;
					
					const auto& in_style = GetStyle();
					auto& out_style = TabLabelStyle::style;

					using col = TabLabelStyle::Colors;

					out_style.rounding = 0;
					out_style.closeButtonRounding = 0;
					out_style.closeButtonBorderWidth = 0;
					out_style.colors[col::Col_TabLabel] = 0;
					out_style.colors[col::Col_TabLabelHovered] = GetColorU32(in_style.Colors[ImGuiCol_ButtonHovered]);
					out_style.colors[col::Col_TabLabelActive] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
					out_style.colors[col::Col_TabLabelText] = GetColorU32(in_style.Colors[ImGuiCol_Text]);
					out_style.colors[col::Col_TabLabelSelected] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
					out_style.colors[col::Col_TabLabelSelectedHovered] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
					out_style.colors[col::Col_TabLabelSelectedActive] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
					out_style.colors[col::Col_TabLabelSelectedText] = GetColorU32(in_style.Colors[ImGuiCol_Text]);
					out_style.colors[col::Col_TabLabelCloseButtonHovered] = GetColorU32(in_style.Colors[ImGuiCol_CloseButtonHovered]);
					out_style.colors[col::Col_TabLabelCloseButtonActive] = GetColorU32(in_style.Colors[ImGuiCol_CloseButtonActive]);
				}

				// Tab algorithm i/o

				auto selected_index = static_cast<int>(signi.current_index);
				int closed_tab_index{ -1 };

				auto ordering = [&]() {
					std::vector<int> out;
					out.resize(signi.folders.size());

					for (int i = 0; i < out.size(); ++i) {
						out[i] = i;
					}

					return out;
				}();

				{
					const auto tab_names = [&]() {
						std::vector<std::string> out;
						out.reserve(signi.folders.size());

						for (const auto& it : signi.folders) {
							auto p = it.get_display_path();

							if (it.has_unsaved_changes()) {
								p += " *";
							}

							out.push_back(p);
						}

						return out;
					}();

					auto tab_names_cstrs = [&]() {
						std::vector<const char*> out;
						out.reserve(signi.folders.size());

						for (const auto& it : tab_names) {
							out.push_back(it.c_str());
						}

						return out;
					}();

					auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 2; return padding; }());
					ImGui::TabLabels(static_cast<int>(signi.folders.size()), tab_names_cstrs.data(), selected_index, nullptr, false, nullptr, ordering.data(), true, true, &closed_tab_index, nullptr);
				}

				/* Read back */

				{
					if (closed_tab_index != -1) {
						close_folder(static_cast<folder_index>(closed_tab_index));
					}
					else {
						bool changed_order = false;

						for (std::size_t i = 0; i < ordering.size(); ++i) {
							if (ordering[i] != i) {
								changed_order = true;
								break;
							}
						}

						auto index_to_set = static_cast<folder_index>(selected_index);

						if (changed_order) {
							decltype(signi.folders) new_tabs;
							new_tabs.reserve(signi.folders.size());

							for (const auto o : ordering) {
								if (o == selected_index) {
									index_to_set = static_cast<folder_index>(new_tabs.size());
								}

								new_tabs.push_back(std::move(signi.folders[o]));
							}

							signi.folders = std::move(new_tabs);
						}

						set_current(index_to_set);
					}
				}
			}
		}
	}

	if (anything_opened()) {
		if (show_summary) {
			auto summary = scoped_window("Summary", &show_summary, ImGuiWindowFlags_AlwaysAutoResize);

			if (anything_opened()) {
				text(typesafe_sprintf("Folder path: %x", folder().current_path));
				//text("Tick rate: %x/s", get_viewed_cosmos().get_solvable().get_steps_per_second()));
				text("Cursor: %x", world_cursor_pos);
				
				const auto printed_camera = get_custom_camera() ? *get_custom_camera() : current_cone;
				
				text("View center: %x", printed_camera.transform.pos);

				text(typesafe_sprintf("Zoom: %x", printed_camera.zoom * 100.f) + " %");

				text("Total entities: %x",
					get_viewed_cosmos().get_entities_count()
				);

				text("World time: %x (%x steps at %x Hz)",
					standard_format_seconds(get_viewed_cosmos().get_total_seconds_passed()),
					get_viewed_cosmos().get_total_steps_passed(),
					1.0f / get_viewed_cosmos().get_fixed_delta().in_seconds()
				);

				text(L"Currently controlling: %x",
					get_viewed_character().alive() ? get_viewed_character().get_name() : L"no entity"
				);
			}
		}

		if (player.show) {
			auto player_window = scoped_window("Player", &player.show, ImGuiWindowFlags_AlwaysAutoResize);

			if (ImGui::Button("Play")) {
				play();
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Pause")) {
				pause();
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Stop")) {
				stop();
			}
		}

		if (show_common_state) {
			auto common = scoped_window("Common", &show_common_state, ImGuiWindowFlags_AlwaysAutoResize);
		}

		if (show_entities) {
			auto entities = scoped_window("Entities", &show_entities);

			static ImGuiTextFilter filter;
			filter.Draw();
			
			cosmic::for_each_entity(work().world, [&](const auto handle) {
				const auto name = to_string(handle.get_name());
				const auto id = handle.get_id();

				if (filter.PassFilter(name.c_str())) {
					auto scope = scoped_id(id.indirection_index);

					if (auto node = scoped_tree_node(name.c_str())) {
						if (ImGui::Button("Control")) {
							set_locally_viewed(id);
						}
					}
				}

			});
		}

		if (show_go_to_all) {
			auto go_to_all = scoped_window("Go to all", &show_go_to_all);
		}

		if (show_go_to_entity) {
			{
				const auto size = vec2 {
					static_cast<float>(settings.go_to_dialog_width),
					(matching_go_to_entities.size() + 2) * ImGui::GetTextLineHeightWithSpacing()
				};

				set_next_window_rect(
					{
						{ screen_size.x / 2 - size.x / 2, menu_bar_size.y * 2 + 1 },
						size	
					},
					ImGuiCond_Always
				);
			}

			auto go_to_entity = scoped_window(
				"Go to entity", 
				&show_go_to_entity,
			   	ImGuiWindowFlags_NoTitleBar 
				| ImGuiWindowFlags_NoResize 
				| ImGuiWindowFlags_NoScrollbar 
				| ImGuiWindowFlags_NoScrollWithMouse
				| ImGuiWindowFlags_NoMove 
				| ImGuiWindowFlags_NoSavedSettings
			);

			static auto arrow_callback = [](ImGuiTextEditCallbackData* data) {
				LOG_NVPS(data->EventFlag);
				LOG_NVPS(data->Buf);			

				auto& self = *reinterpret_cast<editor_setup*>(data->UserData);

				switch (data->EventFlag) {
					case ImGuiInputTextFlags_CallbackHistory: {
						const auto max_index = self.matching_go_to_entities.size();

						if (max_index == 0) {
							break;
						}
						
						if (data->EventKey == ImGuiKey_UpArrow) {
							if (self.go_to_entities_selected_index == 0) {
								self.go_to_entities_selected_index = max_index - 1;
							}
							else {
								--self.go_to_entities_selected_index; 	
							}
						}
						else if (data->EventKey == ImGuiKey_DownArrow) {
							++self.go_to_entities_selected_index; 	
							self.go_to_entities_selected_index %= max_index;
						}
					}

					break;

					case ImGuiInputTextFlags_CallbackAlways: {
#if TODO_NAMES
						const auto current_input_text = std::string(data->Buf, data->BufTextLen);
			
						const bool should_rebuild = 
							current_input_text != self.last_go_to_entities_input
							|| (current_input_text.empty() && self.matching_go_to_entities.empty())
						;
			
						if (!should_rebuild) {
							break;
						}
			
						self.last_go_to_entities_input = current_input_text;
			
						self.matching_go_to_entities.clear();
						const auto query = to_wstring(current_input_text);
			
						unsigned hits = 0;
							
						for (const auto& lex : self.work().world.get_solvable_inferred().name.get_lexicographic_names()) {
							const auto& name = lex.first;
			
							if (query.empty() || to_lowercase(name).find(query) != std::wstring::npos) {
								++hits;
			
								self.matching_go_to_entities.push_back(lex.second);
			
								if (hits >= self.settings.lines_in_go_to_dialogs) {
									break;
								}
							}
						}	

						if (self.matching_go_to_entities.size() > 0) {
							self.go_to_entities_selected_index %= self.matching_go_to_entities.size();
						}
						else {
							self.go_to_entities_selected_index = 0;
						}
#endif
					}
					break;
					
					case ImGuiInputTextFlags_CallbackCompletion: {
						if (const auto match = self.get_matching_go_to_entity()) {
							const auto name = to_string(match.get_name());

							if (name.length() < data->BufSize) {
								std::copy(name.c_str(), name.c_str() + name.length() + 1, data->Buf);

								data->BufTextLen = name.length();
								data->CursorPos = name.length();
								data->BufDirty = true;
							}
						}

						break;
					}

					default: break;
				}

				return 0;
			};

			text("Go to entity");
			ImGui::SameLine();
			
			bool was_acquired = false;
			
			if (ImGui::GetCurrentWindow()->GetID("##GoToEntityInput") != GImGui->ActiveId) {
				ImGui::SetKeyboardFocusHere();
				was_acquired = true;
			}
			
        	if (input_text<256>(
				"##GoToEntityInput", 
				go_to_entity_query,
					ImGuiInputTextFlags_CallbackHistory 
					| ImGuiInputTextFlags_EnterReturnsTrue 
					| ImGuiInputTextFlags_CallbackAlways
					| ImGuiInputTextFlags_CallbackCompletion
				, 
				arrow_callback,
				reinterpret_cast<void*>(this)
			)) {
				const auto selected_name = to_wstring(go_to_entity_query);

				// LOG(selected_name);
				if (const auto match = get_matching_go_to_entity()) {	
					view().selected_entities = { match };

					if (!view().panned_camera.has_value()) {
						view().panned_camera = current_cone;
					}

					if (const auto transform = match.find_logic_transform()) {
						view().panned_camera->transform.pos = transform->pos;
					}
					else {
						LOG("WARNING: transform of %x could not be found.", match);
					}
				}
			}
	
			{
				const auto last_input = to_wstring(last_go_to_entities_input);

				unsigned go_to_index = 0;

				for (const auto& m : matching_go_to_entities) {
					if (go_to_index == go_to_entities_selected_index) {
						bool s = true;
						ImGui::PushID(static_cast<int>(go_to_index));
						ImGui::Selectable("##gotoentity", &s);
						ImGui::PopID();
						ImGui::SameLine(0.f, 0.f);
					}

					const auto name = work().world[m].get_name();
					const auto matched_name = to_lowercase(name);

					const auto unmatched_left = matched_name.find(last_input);
					const auto unmatched_right = unmatched_left + last_input.length();

					text(name.substr(0, unmatched_left));
					ImGui::SameLine(0.f, 0.f);

					{
						auto scope = scoped_style_color(ImGuiCol_Text, ImVec4(0.f, 1.f, 0.f, 1.f));
						text(name.substr(unmatched_left, unmatched_right - unmatched_left));
						ImGui::SameLine(0.f, 0.f);
					}

					text(name.substr(unmatched_right));

					ImGui::SameLine(settings.go_to_dialog_width * 0.7);

					text("(guid: %x)", m);

					++go_to_index;
				}
			}

			if (!was_acquired && ImGui::GetCurrentWindow()->GetID("##GoToEntityInput") != GImGui->ActiveId) {
				show_go_to_entity = false;
			}
		}
	}

	if (open_folder_dialog.valid() && is_ready(open_folder_dialog)) {
		const auto result_path = open_folder_dialog.get();

		if (result_path) {
			open_folder_in_new_tab(path_op(*result_path));
		}
	}

	if (save_project_dialog.valid() && is_ready(save_project_dialog)) {
		if (const auto result_path = save_project_dialog.get()) {
			if (::is_untitled_path(*result_path)) {
				set_popup({"Error", "Can't save to a directory with untitled projects.", ""});
			}
			else {
				const auto previous_path = folder().current_path;
				const bool was_untitled = folder().is_untitled();

				save_current_folder_to(path_op(*result_path));

				if (was_untitled) {
					augs::remove_directory(previous_path);
				}
			}
		}
	}

	if (ok_only_popup) {
		auto& p = *ok_only_popup;

		if (!ImGui::IsPopupOpen(p.title.c_str())) {
			ImGui::OpenPopup(p.title.c_str());
		}

		if (auto popup = scoped_modal_popup(p.title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			text(p.message);

			{
				if (p.details.size() > 0) {
					auto& f = p.details_expanded;

					if (ImGui::Button(f ? "Hide details" : "Show details")) {
						f = !f;
					}

					if (f) {
						text(p.details);
					}
				}
			}

			if (ImGui::Button("OK", ImVec2(120, 0))) { 
				ImGui::CloseCurrentPopup();
				ok_only_popup = std::nullopt;
			}
		}
	}
}

void editor_setup::finish_rectangular_selection() {
	if (anything_opened()) {
		decltype(view().selected_entities) new_selections;

		for_each_selected_entity(
			[&](const auto e) {
				new_selections.emplace(e);
			}
		);

		view().selected_entities = new_selections;

		rectangular_drag_origin = std::nullopt;
		in_rectangular_selection.clear();
	}
}

void editor_setup::clear_all_selections() {
	if (anything_opened()) {
		view().selected_entities.clear();
		in_rectangular_selection.clear();
	}
}

bool editor_setup::escape() {
	if (ok_only_popup) {
		ok_only_popup = std::nullopt;
		return true;
	}
	else if(!player.paused) {
		player.paused = true;
		return true;
	}

	return false;
}

bool editor_setup::confirm_modal_popup() {
	if (ok_only_popup) {
		ok_only_popup = std::nullopt;
		return true;
	}

	return false;
}

void editor_setup::open(const augs::window& owner) {
	if (ok_only_popup) {
		return;
	}

	open_folder_dialog = std::async(
		std::launch::async,
		[&](){
			return owner.choose_directory_dialog("Open folder with project files");
		}
	);
}

void editor_setup::save(sol::state& lua, const augs::window& owner) {
	if (!anything_opened()) {
		return;
	}

	if (folder().is_untitled()) {
		save_as(owner);
	}
	else {
		save_current_folder();
	}
}

void editor_setup::save_as(const augs::window& owner) {
	if (!anything_opened() || ok_only_popup) {
		return;
	}

	save_project_dialog = std::async(
		std::launch::async,
		[&](){
			return owner.choose_directory_dialog("Choose folder for project files");
		}
	);
}

void editor_setup::undo() {
	folder().history.undo(folder());
}

void editor_setup::redo() {
	folder().history.redo(folder());
}

void editor_setup::copy() {

}

void editor_setup::cut() {

}

void editor_setup::paste() {

}

void editor_setup::del() {
	if (anything_opened()) {
		delete_entities_command command;

		for_each_selected_entity(
			[&](const auto e) {
				command.push_entry(work().world[e]);
			}
		);

		if (!command.empty()) {
			folder().history.execute_new(std::move(command), folder());
			clear_all_selections();
		}
	}
}

void editor_setup::go_to_all() {
	show_go_to_all = true;
}

void editor_setup::go_to_entity() {
	show_go_to_entity = true;
	go_to_entity_query.clear();
	matching_go_to_entities.clear();
	last_go_to_entities_input.clear();
	go_to_entities_selected_index = 0;
	ImGui::SetWindowFocus("Go to entity");
}

void editor_setup::reveal_in_explorer(const augs::window& owner) {
	owner.reveal_in_explorer(folder().get_paths().int_file);
}

void editor_setup::play() {
	player.paused = false;
}

void editor_setup::pause() {
	player.paused = true;
}

void editor_setup::play_pause() {
	bool& f = player.paused;
	f = !f;
}

void editor_setup::stop() {
	player.paused = true;
}

void editor_setup::prev() {
	player.paused = true;
}

void editor_setup::next() {
	player.paused = true;
}

void editor_setup::new_tab() {
	try_to_open_new_folder([&](editor_folder& t) {
		const auto new_path = get_first_free_untitled_path("Project%x");
		augs::create_directories(augs::path_type(new_path) += "/");
		t.current_path = new_path;

		/* 
			Initial write-out so that we have some empty intercosms
			in the folder to be loaded later on restart
		*/

		t.save_folder();
	});
}

void editor_setup::next_tab() {
	if (anything_opened()) {
		set_current((signi.current_index + 1) % signi.folders.size());
	}
}

void editor_setup::prev_tab() {
	if (anything_opened()) {
		set_current(signi.current_index == 0 ? static_cast<folder_index>(signi.folders.size() - 1) : signi.current_index - 1);
	}
}

void editor_setup::close_folder(const folder_index i) {
	auto& folder_to_close = signi.folders[i];

	if (folder_to_close.has_unsaved_changes()) {
		set_popup({ "Error", "Nie", "Nie" });
		return;
	}
		
	if (folder_to_close.is_untitled()) {
		augs::remove_directory(folder_to_close.current_path);
	}

	signi.folders.erase(signi.folders.begin() + i);

	if (signi.folders.empty()) {
		signi.current_index = -1;
		pause();
	}
	else {
		signi.current_index = std::min(signi.current_index, static_cast<folder_index>(signi.folders.size() - 1));
	}
}

void editor_setup::close_folder() {
	if (anything_opened()) {
		close_folder(signi.current_index);
	}
}

bool editor_setup::handle_input_before_imgui(
	const augs::event::state& common_input_state,
	const augs::event::change e,

	augs::window& window,
	sol::state& lua
) {
	using namespace augs::event;
	using namespace keys;

	if (settings.autosave.on_lost_focus && e.msg == message::deactivate) {
		force_autosave_now();
	}

	if (e.was_any_key_pressed()) {
		const auto k = e.data.key.key;
		
		/* Media buttons work regardless of pause */

		switch (k) {
			case key::PLAY_PAUSE_TRACK: play_pause(); return true;
			case key::PREV_TRACK: prev(); return true;
			case key::NEXT_TRACK: next(); return true;
			case key::STOP_TRACK: stop(); return true;
			default: break;
		}

		if (player.paused) {
			const bool has_ctrl{ common_input_state[key::LCTRL] };
			const bool has_shift{ common_input_state[key::LSHIFT] };

			if (has_ctrl) {
				if (has_shift) {
					switch (k) {
						case key::E: reveal_in_explorer(window); return true;
						case key::TAB: prev_tab(); return true;
						case key::F5: fill_with_minimal_scene(lua); return true;
						default: break;
					}
				}

				switch (k) {
					case key::S: save(lua, window); return true;
					case key::O: open(window); return true;
					case key::COMMA: go_to_all(); return true;
					case key::N: new_tab(); return true;
					case key::W: close_folder(); return true;
					case key::TAB: next_tab(); return true;
					default: break;
				}
			}

			if (has_shift) {
				switch (k) {
					case key::F5: fill_with_test_scene(lua); return true;
					default: break;
				}
			}

			switch (k) {
				case key::F12: save_as(window); return true;
				case key::ENTER: return confirm_modal_popup();
				default: break;
			}
		}
	}

	return false;
}


bool editor_setup::handle_unfetched_window_input(
	const augs::event::state& common_input_state,
	const augs::event::change e,

	augs::window& window,
	sol::state& lua,
	
	const camera_cone current_cone
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto mouse_pos = vec2i(ImGui::GetIO().MousePos);
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);
	const auto world_cursor_pos = current_cone.to_world_space(screen_size, mouse_pos);
	const auto world_screen_center = current_cone.to_world_space(screen_size, screen_size/2);

	if (!anything_opened()) {
		return false;
	}

	if (player.paused) {
		const bool has_ctrl{ common_input_state[key::LCTRL] || common_input_state[key::RCTRL] };
		const bool has_shift{ common_input_state[key::LSHIFT] };

		const auto pan_mult = [&](){
			float result = 1.f;

			if (has_ctrl) {
				result *= 5;
			}

			if (has_shift) {
				result /= 5;
			}

			return result;
		}();

		const auto zoom_mult = [&](){
			float result = 1.f;

			if (has_ctrl) {
				result *= 5;
			}

			if (has_shift) {
				result /= 5;
			}

			return result;
		}();

		auto pan_scene = [&](const auto amount) {
			if (!view().panned_camera.has_value()) {
				view().panned_camera = current_cone;
			}

			auto& camera = *view().panned_camera;

			camera.transform.pos -= pan_mult * amount / camera.zoom;
		};

		auto zoom_scene = [&](const auto zoom_amount, const auto zoom_point) {
			if (!view().panned_camera.has_value()) {
				view().panned_camera = current_cone;
			}

			auto& camera = *view().panned_camera;

			const auto old_zoom = camera.zoom;
			const auto zoom_offset = 0.09f * old_zoom * zoom_amount * zoom_mult;
			const auto new_zoom = std::clamp(old_zoom + zoom_offset, 0.01f, 10.f);

			camera.zoom = new_zoom;	
			camera.transform.pos += (1 - 1 / (new_zoom/old_zoom))*(zoom_point - camera.transform.pos);
		};

		if (e.msg == message::wheel) {
			zoom_scene(e.data.scroll.amount, world_cursor_pos);
		}

		if (e.msg == message::mousemotion) {
			if (common_input_state[key::RMOUSE]) {
				pan_scene(vec2(e.data.mouse.rel) * settings.camera_panning_speed);
				return true;
			}
			else {
				hovered_entity = {};

				{
					const auto drag_dead_area = 3.f;
					const auto drag_offset = world_cursor_pos - last_ldown_position;

					if (common_input_state[key::LMOUSE] && !drag_offset.is_epsilon(drag_dead_area)) {
						rectangular_drag_origin = last_ldown_position;
						held_entity = {};
					}
				}

				if (rectangular_drag_origin.has_value()) {
					auto world_range = ltrb::from_points(*rectangular_drag_origin, world_cursor_pos);

					in_rectangular_selection.clear();

					const auto query = visible_entities_query{
						work().world,
						{ world_range.get_center(), 1.f },
						world_range.get_size()
					};

					in_rectangular_selection.acquire_non_physical(query);
					in_rectangular_selection.acquire_physical(query);
				}
				else {
					hovered_entity = get_hovered_world_entity(
						work().world, 
						world_cursor_pos, 
						[&](const entity_id id) { 
							if (work().world[id].has<components::wandering_pixels>()) {
								return false;
							}

							return true; 
						}
					);
				}
			}
		}

		if (e.was_pressed(key::SLASH)) {
			go_to_entity();
			return true;
		}
		else if (e.was_pressed(key::LMOUSE)) {
			const bool has_ctrl{ common_input_state[key::LCTRL] };

			const auto world_cursor_pos = current_cone.to_world_space(
				window.get_screen_size(),
				common_input_state.mouse.pos
			);

			last_ldown_position = world_cursor_pos;
			held_entity = hovered_entity;

			auto& selections = view().selected_entities;

			if (/* new_selection */ !has_ctrl) {
				selections.clear();
			}

			return true;
		}
		else if (e.was_released(key::LMOUSE)) {
			const auto held = work().world[held_entity];

			if (held.alive()) {
				const bool has_ctrl{ common_input_state[key::LCTRL] };
				auto& selections = view().selected_entities;

				if (has_ctrl && found_in(selections, held)) {
					selections.erase(held);
				}
				else {
					selections.emplace(hovered_entity);
				}
			}

			held_entity = {};

			finish_rectangular_selection();
		}

		if (e.was_any_key_pressed()) {
			const auto k = e.data.key.key;

			if (has_ctrl) {
				if (has_shift) {
					switch (k) {
						case key::Z: redo(); return true;
						default: break;
					}
				}

				switch (k) {
					case key::Z: undo(); return true;
					case key::C: copy(); return true;
					case key::X: cut(); return true;
					case key::V: paste(); return true;
					default: break;
				}
			}

			const auto key_pan_amount = 50.f;
			const auto key_zoom_amount = 1.f;

			switch (k) {
				case key::C: 
					if (view().selected_entities.size() == 1) { 
						set_locally_viewed(*view().selected_entities.begin()); 
						return true;
					}
				case key::I: play(); return true;
				case key::DEL: del(); return true;
				case key::HOME: view().panned_camera = std::nullopt; return true;
				case key::UP: pan_scene(vec2(0, key_pan_amount)); return true;
				case key::DOWN: pan_scene(vec2(0, -key_pan_amount)); return true;
				case key::RIGHT: pan_scene(vec2(-key_pan_amount, 0)); return true;
				case key::LEFT: pan_scene(vec2(key_pan_amount, 0)); return true;
				case key::MINUS: zoom_scene(-key_zoom_amount, world_screen_center); return true;
				case key::EQUAL: zoom_scene(key_zoom_amount, world_screen_center); return true;
				default: break;
			}
		}
	}

	return false;
}