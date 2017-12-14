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

void editor_setup::on_tab_changed() {
	hovered_entity = {};
	player_paused = true;
	finish_rectangular_selection();
}

void editor_setup::set_popup(const editor_popup p) {
	current_popup = p;
}

void editor_setup::set_locally_viewed(const entity_id id) {
	work().locally_viewed = id;
	tab().panned_camera = std::nullopt;
}

std::optional<editor_popup> open_intercosm(intercosm& work, const intercosm_path_op op) {
	if (op.path.empty()) {
		return std::nullopt;
	}

	try {
		work.open(op);
	}
	catch (const intercosm_loading_error err) {
		editor_popup p;

		p.title = err.title;
		p.details = err.details;
		p.message = err.message;

		return p;
	}

	return std::nullopt;
}


editor_setup::editor_setup(
	sol::state& lua
) : 
	destructor_autosave_input{ lua },
	recent(lua) 
{
	augs::create_directories(get_untitled_dir());

	open_last_tabs(lua);
}

editor_setup::editor_setup(
	sol::state& lua, 
	const augs::path_type& intercosm_path
) : 
	destructor_autosave_input{ lua },
	recent(lua) 
{
	augs::create_directories(get_untitled_dir());

	open_last_tabs(lua);
	open_intercosm_in_new_tab({ lua, intercosm_path });
}

editor_setup::~editor_setup() {
	autosave(destructor_autosave_input);
}

void editor_setup::open_last_tabs(sol::state& lua) {
	ensure(tabs.empty());
	ensure(works.empty());

	try {
		const auto opened_tabs = augs::load_from_lua_table<editor_saved_tabs>(lua, get_editor_tabs_path());
		tabs = opened_tabs.tabs;

		if (!tabs.empty()) {
			/* Reload intercosms */

			std::vector<tab_index_type> tabs_to_close;

			for (std::size_t i = 0; i < tabs.size(); ++i) {
				works.emplace_back(std::make_unique<intercosm>());
				
				if (const bool try_loading_unsaved = !tabs[i].is_untitled()) {
					const auto unsaved_path = get_unsaved_path(tabs[i].current_path);

					if (const auto popup = open_intercosm(*works.back(), { lua, unsaved_path })) {
						if (const auto popup = open_intercosm(*works.back(), { lua, tabs[i].current_path })) {
							set_popup(*popup);
							tabs_to_close.push_back(static_cast<tab_index_type>(i));
						}
					}
				}
				else {
					if (const auto popup = open_intercosm(*works.back(), { lua, tabs[i].current_path })) {
						set_popup(*popup);
						tabs_to_close.push_back(static_cast<tab_index_type>(i));
					}
				}
			}
			
			set_current_tab(opened_tabs.current_tab_index);

			sort_range(tabs_to_close);

			for (const auto i : reverse(tabs_to_close)) {
				close_tab(i);
			}
		}
	}
	catch (...) {

	}
}

void editor_setup::autosave(const autosave_input in) const {
	editor_saved_tabs saved_tabs;

	auto& lua = in.lua;

	if (has_current_tab()) {
		saved_tabs.tabs = tabs;
		saved_tabs.current_tab_index = current_index;

		for (std::size_t i = 0; i < tabs.size(); ++i) {
			const auto& t = tabs[i];
			const auto& w = *works[i];

			if (const bool resave_untitled_work = t.is_untitled()) {
				const auto saving_path = t.current_path;
				w.save({ lua, saving_path });
			}
			else if (const bool cache_changes_of_named_work = t.has_unsaved_changes()) {
				auto extension = t.current_path.extension();
				const auto saving_path = augs::path_type(t.current_path).replace_extension(extension += ".unsaved");
				w.save({ lua, saving_path });
			}
		}
	}

	augs::save_as_lua_table(lua, saved_tabs, get_editor_tabs_path());
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
	if (has_current_tab()) {
		config.window.name = "Editor - " + tab().get_display_path();
	}
	else {
		config.window.name = "Editor";
	}

	if (player_paused) {
		config.drawing.draw_aabb_highlighter = false;
	}

	return;
}

void editor_setup::apply(const config_lua_table& cfg) {
	if (cfg.editor.autosave != settings.autosave) {
		autosave_timer = {};
	}
		
	settings = cfg.editor;

	return;
}

bool editor_setup::open_intercosm_in_new_tab(const path_operation op) {
	for (std::size_t i = 0; i < tabs.size(); ++i) {
		if (tabs[i].current_path == op.path) {
			set_current_tab(static_cast<tab_index_type>(i));
			return true;
		}
	}

	return try_to_open_new_tab(
		[this, op](editor_tab& t, intercosm& work) {
			if (const auto popup = open_intercosm(work, op)) {
				set_popup(*popup);
				return false;
			}
			
			t.set_intercosm_path(op.lua, op.path, recent);

			return true;
		}
	);
}

void editor_setup::save_current_tab_to(const path_operation op) {
	work().save(op);
	tab().set_intercosm_path(op.lua, op.path, recent);
}

void editor_setup::fill_with_minimal_scene(sol::state& lua) {
#if BUILD_TEST_SCENES
	if (has_current_tab()) {
		work().make_test_scene(lua, true);

		clear_all_selections();
	}
#endif
}

void editor_setup::fill_with_test_scene(sol::state& lua) {
#if BUILD_TEST_SCENES
	if (has_current_tab()) {
		work().make_test_scene(lua, false);

		clear_all_selections();
	}
#endif
}

static auto get_filters() {
	return std::vector<augs::window::file_dialog_filter> {
		{ "Hypersomnia intercosm file (*.int)", ".int" },
		{ "Hypersomnia compatibile intercosm file (*.lua)", ".lua" }
	};
}

void editor_setup::perform_custom_imgui(
	sol::state& lua,
	augs::window& owner,
	const bool in_direct_gameplay,

	const camera_cone current_cone
) {
	using namespace augs::imgui;

	if (
		settings.autosave.enabled 
		&& settings.autosave.once_every_min <= autosave_timer.get<std::chrono::minutes>()
	) {
		autosave({ lua });
		autosave_timer.reset();
	}

	auto in_path = [&](const auto& path) {
		return path_operation{ lua, path };
	};

	auto item_if_tabs_and = [this](const bool condition, const char* label, const char* shortcut = nullptr) {
		return ImGui::MenuItem(label, shortcut, nullptr, condition && has_current_tab());
	};

	auto item_if_tabs = [&](const char* label, const char* shortcut = nullptr) {
		return item_if_tabs_and(true, label, shortcut);
	};

	const auto mouse_pos = vec2i(ImGui::GetIO().MousePos);
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);
	const auto world_cursor_pos = current_cone.to_world_space(screen_size, mouse_pos);

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
								open_intercosm_in_new_tab(in_path(target_path));
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
						if (has_current_tab()) {
							return std::string("Close ") + tab().get_display_path();
						}

						return "Close";
					}();

					if (item_if_tabs(close_str.c_str(), "CTRL+W")) {
						close_tab();
					}

					if (item_if_tabs("Close all")) {
						while (has_current_tab()) {
							close_tab();
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
					if (item_if_tabs("Fill with minimal scene", "SHIFT+F5")) {
						fill_with_minimal_scene(lua);
					}

					if (item_if_tabs("Fill with test scene")) {
						fill_with_test_scene(lua);
					}
#else
					if (item_if_tabs_and(false, "Fill with minimal scene", "SHIFT+F5")) {}
					if (item_if_tabs_and(false, "Fill with test scene")) {}
#endif
				}
				if (auto menu = scoped_menu("View")) {
					if (item_if_tabs("Summary")) {
						show_summary = true;
					}
					if (item_if_tabs("Player")) {
						show_player = true;
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

		if (has_current_tab()) {
			const auto& g = *ImGui::GetCurrentContext();
			const auto bar_size = ImVec2(g.IO.DisplaySize.x, g.FontBaseSize + g.Style.FramePadding.y * 2.0f);

			if (const auto tab_menu = scoped_tab_menu_bar(bar_size.y)) {
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

				auto selected_index = static_cast<int>(current_index);
				int closed_tab_index{ -1 };

				auto ordering = [&]() {
					std::vector<int> out;
					out.reserve(tabs.size());

					for (const auto& it : tabs) {
						out.push_back(static_cast<int>(out.size()));
					}

					return out;
				}();

				{
					const auto tab_names = [&]() {
						std::vector<std::string> out;
						out.reserve(tabs.size());

						for (const auto& it : tabs) {
							out.push_back(it.get_display_path());
						}

						return out;
					}();

					auto tab_names_cstrs = [&]() {
						std::vector<const char*> out;
						out.reserve(tabs.size());

						for (const auto& it : tab_names) {
							out.push_back(it.c_str());
						}

						return out;
					}();

					auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 2; return padding; }());
					ImGui::TabLabels(static_cast<int>(tabs.size()), tab_names_cstrs.data(), selected_index, nullptr, false, nullptr, ordering.data(), true, true, &closed_tab_index, nullptr);
				}

				/* Read back */

				{
					if (closed_tab_index != -1) {
						close_tab(static_cast<tab_index_type>(closed_tab_index));
					}
					else {
						bool changed_order = false;

						for (std::size_t i = 0; i < ordering.size(); ++i) {
							if (ordering[i] != i) {
								changed_order = true;
								break;
							}
						}

						auto index_to_set = static_cast<tab_index_type>(selected_index);

						if (changed_order) {
							decltype(tabs) new_tabs;
							decltype(works) new_works;

							new_tabs.reserve(tabs.size());
							new_works.reserve(works.size());

							for (const auto o : ordering) {
								if (o == selected_index) {
									index_to_set = static_cast<tab_index_type>(new_tabs.size());
								}

								new_tabs.push_back(tabs[o]);
								new_works.push_back(std::move(works[o]));
							}

							tabs = new_tabs;
							works = std::move(new_works);
						}

						set_current_tab(index_to_set);
					}
				}
			}
		}
	}

	if (has_current_tab()) {
		if (show_summary) {
			auto summary = scoped_window("Summary", &show_summary, ImGuiWindowFlags_AlwaysAutoResize);

			if (has_current_tab()) {
				//text("Tick rate: %x/s", get_viewed_cosmos().get_steps_per_second()));
				text("Cursor: %x", world_cursor_pos);
				
				const auto printed_camera = get_custom_camera() ? *get_custom_camera() : current_cone;
				
				text("View center: %x", printed_camera.transform.pos);

				text(typesafe_sprintf("Zoom: %x", printed_camera.zoom * 100.f) + " %");

				text("Total entities: %x/%x",
					get_viewed_cosmos().get_entities_count(),
					get_viewed_cosmos().get_maximum_entities()
				);

				text("World time: %x (%x steps)",
					standard_format_seconds(get_viewed_cosmos().get_total_seconds_passed()),
					get_viewed_cosmos().get_total_steps_passed()
				);

				text(L"Currently controlling: %x",
					get_viewed_character().alive() ? get_viewed_character().get_name() : L"no entity"
				);
			}
		}

		if (show_player) {
			auto player = scoped_window("Player", &show_player, ImGuiWindowFlags_AlwaysAutoResize);

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
			
			work().world.for_each_entity_id([&](const entity_id id) {
				const auto handle = work().world[id];
				const auto name = to_string(handle.get_name());

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
			center_next_window(0.30f);

			auto go_to_entity = scoped_window(
				"Go to entity", 
				&show_go_to_entity,
			   	ImGuiWindowFlags_NoTitleBar 
				| ImGuiWindowFlags_NoResize 
				| ImGuiWindowFlags_NoMove 
				| ImGuiWindowFlags_NoSavedSettings
			);

			static std::array<char, 512> buf {};

			static auto arrow_callback = [](ImGuiTextEditCallbackData* data){
				LOG_NVPS(data->EventFlag);
				LOG_NVPS(data->EventKey);
				switch (data->EventFlag) {
					case ImGuiInputTextFlags_CallbackCompletion: {

						if (data->EventKey == ImGuiKey_UpArrow) {
							LOG("UP arrow");
						}
						else if (data->EventKey == ImGuiKey_DownArrow) {
							LOG("Down arrow");
						}
					}

					break;

					default: break;
				}

				return 0;
			};

			text("Go to entity");
			ImGui::SameLine();

        	if (ImGui::InputText(
					"", 
					buf.data(), 
					buf.size(), 
					ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_EnterReturnsTrue, 
					arrow_callback,
					nullptr
			)) {
				const auto selected_name = to_wstring(std::string(buf.data()));

				LOG(selected_name);

				tab().selected_entities = { *work().world.get_entities_by_name(selected_name).begin() };
			}
		}
	}

	if (open_file_dialog.valid() && is_ready(open_file_dialog)) {
		const auto result_path = open_file_dialog.get();

		if (result_path) {
			open_intercosm_in_new_tab(in_path(*result_path));
		}
	}

	if (save_file_dialog.valid() && is_ready(save_file_dialog)) {
		const auto result_path = save_file_dialog.get();

		if (result_path) {
			const auto extension = augs::path_type(*result_path).extension();

			const auto filters = get_filters();
			bool found_extension = false;

			for (const auto& f : filters) {
				if (f.extension == extension) {
					found_extension = true;
					save_current_tab_to(in_path(*result_path));
					break;
				}
			}

			if (!found_extension) {
				set_popup({ "Error", "Unrecognized extension: " + extension.string() });
			}
		}
	}

	if (current_popup) {
		auto& p = *current_popup;

		if (!ImGui::IsPopupOpen(p.title.c_str())) {
			ImGui::OpenPopup(p.title.c_str());
		}

		if (auto popup = scoped_modal_popup(p.title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			text(p.message);

			{
				auto& f = p.details_expanded;

				if (ImGui::Button(f ? "Hide details" : "Show details")) {
					f = !f;
				}

				if (f) {
					text(p.details);
				}
			}

			if (ImGui::Button("OK", ImVec2(120, 0))) { 
				ImGui::CloseCurrentPopup();
				current_popup = std::nullopt;
			}
		}
	}
}

void editor_setup::finish_rectangular_selection() {
	if (has_current_tab()) {
		decltype(tab().selected_entities) new_selections;

		for_each_selected_entity(
			[&](const auto e) {
				new_selections.emplace(e);
			}
		);

		tab().selected_entities = new_selections;

		rectangular_drag_origin = std::nullopt;
		in_rectangular_selection.clear();
	}
}

void editor_setup::clear_all_selections() {
	if (has_current_tab()) {
		tab().selected_entities.clear();
		in_rectangular_selection.clear();
	}
}

bool editor_setup::escape() {
	if (current_popup) {
		current_popup = std::nullopt;
		return true;
	}

	return false;
}

bool editor_setup::confirm_modal_popup() {
	if (current_popup) {
		current_popup = std::nullopt;
		return true;
	}

	return false;
}

void editor_setup::open(const augs::window& owner) {
	if (current_popup) {
		return;
	}

	open_file_dialog = std::async(
		std::launch::async,
		[&](){
			return owner.open_file_dialog(get_filters(), "Open intercosm");
		}
	);
}

void editor_setup::save(sol::state& lua, const augs::window& owner) {
	if (!has_current_tab()) {
		return;
	}

	if (tab().is_untitled()) {
		save_as(owner);
	}
	else {
		save_current_tab_to({ lua, tab().current_path });
	}
}

void editor_setup::save_as(const augs::window& owner) {
	if (!has_current_tab() || current_popup) {
		return;
	}

	save_file_dialog = std::async(
		std::launch::async,
		[&](){
			return owner.save_file_dialog(get_filters());
		}
	);
}

void editor_setup::undo() {

}

void editor_setup::redo() {

}

void editor_setup::copy() {

}

void editor_setup::cut() {

}

void editor_setup::paste() {

}

void editor_setup::del() {
	if (has_current_tab()) {
		for_each_selected_entity(
			[&](const auto e) {
				work().world.delete_entity(e);
			}
		);

		clear_all_selections();
	}
}

void editor_setup::go_to_all() {
	show_go_to_all = true;
}

void editor_setup::go_to_entity() {
	show_go_to_entity = true;
}

void editor_setup::open_containing_folder() {
	if (const auto path_str = augs::path_type(tab().current_path).replace_filename("").string();
		path_str.size() > 0
	) {
		augs::shell(path_str);
	}
	else {
		augs::shell(std::experimental::filesystem::current_path().replace_filename("").string());
	}
}

void editor_setup::play() {
	player_paused = false;
}

void editor_setup::pause() {
	player_paused = true;
}

void editor_setup::play_pause() {
	bool& f = player_paused;
	f = !f;
}

void editor_setup::stop() {
	player_paused = true;
}

void editor_setup::prev() {
	player_paused = true;
}

void editor_setup::next() {
	player_paused = true;
}

void editor_setup::new_tab() {
	try_to_open_new_tab([&](editor_tab& t, intercosm& w) {
		const auto path = get_first_free_untitled_path("Intercosm%x.int");
		augs::save_as_text(path, "empty");

		t.current_path = path;
		return true; 
	});
}

void editor_setup::next_tab() {
	if (has_current_tab()) {
		set_current_tab((current_index + 1) % tabs.size());
	}
}

void editor_setup::prev_tab() {
	if (has_current_tab()) {
		set_current_tab(current_index == 0 ? static_cast<tab_index_type>(tabs.size() - 1) : current_index - 1);
	}
}

void editor_setup::close_tab(const tab_index_type i) {
	auto& tab_to_close = tabs[i];

	if (tab_to_close.has_unsaved_changes()) {
		set_popup({ "Nie", "Nie", "Nie" });
		return;
	}
		
	if (tab_to_close.is_untitled()) {
		augs::remove_file(tab_to_close.current_path);
	}

	tabs.erase(tabs.begin() + i);
	works.erase(works.begin() + i);

	if (!tabs.empty()) {
		set_current_tab(std::min(current_index, static_cast<tab_index_type>(tabs.size() - 1)));
	}
	else {
		unset_current_tab();
		pause();
	}
}

void editor_setup::close_tab() {
	if (has_current_tab()) {
		close_tab(current_index);
	}
}

bool editor_setup::handle_top_level_window_input(
	const augs::event::state& common_input_state,
	const augs::event::change e,

	augs::window& window,
	sol::state& lua
) {
	using namespace augs::event::keys;

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

		if (player_paused) {
			const bool has_ctrl{ common_input_state[key::LCTRL] };
			const bool has_shift{ common_input_state[key::LSHIFT] };

			if (has_ctrl) {
				if (has_shift) {
					switch (k) {
						case key::E: open_containing_folder(); return true;
						case key::TAB: prev_tab(); return true;
						default: break;
					}
				}

				switch (k) {
					case key::S: save(lua, window); return true;
					case key::O: open(window); return true;
					case key::COMMA: go_to_all(); return true;
					case key::N: new_tab(); return true;
					case key::W: close_tab(); return true;
					case key::TAB: next_tab(); return true;
					default: break;
				}
			}

			if (has_shift) {
				switch (k) {
					case key::F5: fill_with_minimal_scene(lua); return true;
				}
			}

			switch (k) {
				case key::F12: save_as(window); return true;
				case key::SLASH: go_to_entity(); return true;
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

	if (player_paused) {
		if (e.msg == message::wheel) {
			if (has_current_tab()) {
				if (!tab().panned_camera.has_value()) {
					tab().panned_camera = current_cone;
				}

				auto& camera = *tab().panned_camera;

				const auto old_zoom = camera.zoom;
				const auto zoom_offset = 0.09f * camera.zoom * e.data.scroll.amount;
				camera.zoom = std::clamp(camera.zoom + zoom_offset, 0.01f, 10.f);
				const auto new_zoom = camera.zoom;
				const auto zoom_point = world_cursor_pos;
				
				camera.transform.pos += (1 - 1 / (new_zoom/old_zoom))*(zoom_point - camera.transform.pos);
			}
		}

		if (e.msg == message::mousemotion) {
			if (common_input_state[key::RMOUSE]) {
				if (has_current_tab()) {
					if (!tab().panned_camera.has_value()) {
						tab().panned_camera = current_cone;
					}
					
					auto& camera = *tab().panned_camera;

					camera.transform.pos -= vec2(e.data.mouse.rel) * (settings.camera_panning_speed / camera.zoom);

					return true;
				}
			}
			else {
				hovered_entity = {};

				if (has_current_tab()) {
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
		}

		if (e.was_pressed(key::LMOUSE)) {
			if (has_current_tab()) {
				const bool has_ctrl{ common_input_state[key::LCTRL] };

				const auto world_cursor_pos = current_cone.to_world_space(
					window.get_screen_size(),
					common_input_state.mouse.pos
				);

				last_ldown_position = world_cursor_pos;
				held_entity = hovered_entity;

				auto& selections = tab().selected_entities;

				if (const bool new_selection = !has_ctrl) {
					selections.clear();
				}

				return true;
			}
		}
		else if (e.was_released(key::LMOUSE)) {
			if (has_current_tab()) {
				const auto held = work().world[held_entity];

				if (held.alive()) {
					const bool has_ctrl{ common_input_state[key::LCTRL] };
					auto& selections = tab().selected_entities;

					if (has_ctrl && found_in(selections, held)) {
						selections.erase(held);
					}
					else {
						selections.emplace(hovered_entity);
					}
				}
			}

			held_entity = {};

			finish_rectangular_selection();
		}

		if (e.was_pressed(key::HOME)) {
			if (has_current_tab()) {
				tab().panned_camera = std::nullopt;

				return true;
			}
		}

		if (e.was_any_key_pressed()) {
			const auto k = e.data.key.key;

			const bool has_ctrl{ common_input_state[key::LCTRL] };
			const bool has_shift{ common_input_state[key::LSHIFT] };

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

			switch (k) {
			case key::DEL: del(); return true;
			default: break;
			}
		}
	}

	return false;
}