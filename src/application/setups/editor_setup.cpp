#include "augs/templates/string_templates.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/addons/imguitabwindow/imguitabwindow.h"
#include "augs/readwrite/lua_readwrite.h"
#include "augs/filesystem/file.h"
#include "augs/templates/thread_templates.h"
#include "augs/templates/chrono_templates.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/shell.h"

#include "application/config_lua_table.h"
#include "application/setups/editor_setup.h"

#include <imgui/imgui_internal.h>

#include "generated/introspectors.h"

editor_tab::editor_tab(std::size_t horizontal_index) 
	: horizontal_index(horizontal_index) 
{}

std::string editor_tab::get_display_path()  const {
	if (untitled_index) {
		return typesafe_sprintf("Workspace-%x.wp", *untitled_index);
	}
	
	return current_path.filename().string();
}

void editor_setup::set_popup(const editor_popup p) {
	current_popup = p;
}

void editor_tab::set_workspace_path(const path_operation op) {
	untitled_index = std::nullopt;

	current_path = op.path;
	op.recent.add(op.lua, op.path);
}

void editor_tab::set_locally_viewed(const entity_id id) {
	work.locally_viewed = id;
	panning = {};
}

std::optional<editor_popup> editor_tab::open_workspace(const path_operation op) {
	if (op.path.empty()) {
		return std::nullopt;
	}

	const auto display_path = augs::to_display_path(op.path);

	try {
		if (op.path.extension() == ".wp") {
			augs::load(work, op.path);
		}
		else if (op.path.extension() == ".lua") {
			augs::load_from_lua_table(op.lua, work, op.path);
		}
		else {
			return std::nullopt;
		}

		set_workspace_path(op);
	}
	catch (const cosmos_loading_error err) {
		return { {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile(s) might be corrupt.", display_path),
			err.what()
		} };
	}
	catch (const augs::stream_read_error err) {
		return { {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile(s) might be corrupt.", display_path),
			err.what()
		} };
	}
	catch (const augs::lua_deserialization_error err) {
		return { {
			"Error",
			typesafe_sprintf("Failed to load %x.\nNot a valid lua table.", display_path),
			err.what()
		} };
	}
	catch (const augs::ifstream_error err) {
		return { {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile(s) might be missing.", display_path),
			err.what()
		} };
	}

	return std::nullopt;
}

void editor_tab::save_workspace(const path_operation op) {
	if (op.path.extension() == ".wp") {
		augs::save(work, op.path);
	}
	else if (op.path.extension() == ".lua") {
		augs::save_as_lua_table(op.lua, work, op.path);
	}

	set_workspace_path(op);
}

void editor_setup::open_untitled_workspace() {
	tab().work.make_blank();
	tab().current_path = {};
}

static auto get_recent_paths_path() {
	return "generated/editor_recent_paths.lua";
}

editor_recent_paths::editor_recent_paths(sol::state& lua) {
	try {
		augs::load_from_lua_table(lua, *this, get_recent_paths_path());
	}
	catch (const augs::ifstream_error) {

	}
	catch (const augs::lua_deserialization_error) {

	}
}

void editor_recent_paths::add(sol::state& lua, const augs::path_type& path) {
	erase_element(paths, path);
	paths.insert(paths.begin(), path);
	augs::save_as_lua_table(lua, *this, get_recent_paths_path());
}

void editor_recent_paths::clear(sol::state& lua) {
	paths.clear();
	augs::save_as_lua_table(lua, *this, get_recent_paths_path());
}

bool editor_recent_paths::empty() const {
	return paths.empty();
}

editor_setup::editor_setup(sol::state& lua) : recent(lua) {}

editor_setup::editor_setup(sol::state& lua, const augs::path_type& workspace_path) : recent(lua) {
	open_workspace({ lua, workspace_path });
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
	if (has_tabs()) {
		config.window.name = "Editor - " + tab().get_display_path();
	}
	else {
		config.window.name = "Editor";
	}

	return;
}

void editor_setup::open_workspace(const path_operation op) {
	if (0 && "MSVC strange error workaround") {
		editor_tab* e = nullptr;
		auto ppp = e->open_workspace({ op.lua, recent, op.path });
	}
	
	try_new_tab(
		[this, op](editor_tab& t) {
			if (const auto popup = t.open_workspace({ op.lua, recent, op.path })) {
				set_popup(*popup);
				return false;
			}
			
			return true;
		}
	);
}

void editor_setup::save_workspace(const path_operation op) {
	tab().save_workspace({ op.lua, recent, op.path });
}

void editor_setup::fill_with_test_scene(sol::state& lua) {
#if BUILD_TEST_SCENES
	if (has_tabs()) {
		tab().work.make_test_scene(lua, false);
	}
#endif
}

void editor_setup::perform_custom_imgui(
	sol::state& lua,
	augs::window& owner,
	const bool in_direct_gameplay
) {
	using namespace augs::imgui;

	auto in_path = [&](const auto& path) {
		return path_operation{ lua, path };
	};

	auto item_if_tabs_and = [this](const bool condition, const char* label, const char* shortcut = nullptr) {
		return ImGui::MenuItem(label, shortcut, nullptr, condition && has_tabs());
	};

	auto item_if_tabs = [&](const char* label, const char* shortcut = nullptr) {
		return item_if_tabs_and(true, label, shortcut);
	};

	if (!in_direct_gameplay) {
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
						IMPORTANT! recent.paths can be altered in the loop by loading a workspace,
						thus we need to copy its contents.
					*/

					const auto recent_paths = recent.paths;

					for (const auto& target_path : recent_paths) {
						const auto str = augs::to_display_path(target_path).string();

						if (ImGui::MenuItem(str.c_str())) {
							open_workspace(in_path(target_path));
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
					if (has_tabs()) {
						return std::string("Close ") + tab().get_display_path();
					}

					return "Close";
				}();

				if (item_if_tabs(close_str.c_str(), "CTRL+W")) {
					close_tab();
				}

				if (item_if_tabs("Close all")) {
					while (has_tabs()) {
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

				if (item_if_tabs_and(BUILD_TEST_SCENES, "Fill with test scene", "SHIFT+F5")) {
					fill_with_test_scene(lua);
				}
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

		if (has_tabs()) {
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
					out_style.colors[col::Col_TabLabelSelected] = GetColorU32(in_style.Colors[ImGuiCol_Button]);
					out_style.colors[col::Col_TabLabelSelectedHovered] = GetColorU32(in_style.Colors[ImGuiCol_Button]);
					out_style.colors[col::Col_TabLabelSelectedActive] = GetColorU32(in_style.Colors[ImGuiCol_Button]);
					out_style.colors[col::Col_TabLabelSelectedText] = GetColorU32(in_style.Colors[ImGuiCol_Text]);
					out_style.colors[col::Col_TabLabelCloseButtonHovered] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
					out_style.colors[col::Col_TabLabelCloseButtonActive] = GetColorU32(rgba{ 210, 0, 0, 255 });
				}

				std::vector<std::string> tab_names;
				std::vector<const char*> tab_names_cstrs;
				std::vector<int> ordering;

				tab_names.reserve(tabs.size());
				tab_names_cstrs.reserve(tabs.size());
				ordering.resize(tabs.size());

				int selected_index = -1;

				for (const auto& it : tabs) {
					auto& t = it.second;

					const auto this_index = tab_names.size();

					if (&t == current_tab) {
						selected_index = static_cast<int>(this_index);
					}

					ordering[t.horizontal_index] = this_index;
					
					tab_names.push_back(t.get_display_path());
				}

				for (const auto& s : tab_names) {
					tab_names_cstrs.push_back(s.c_str());
				}

				int closed_tab_index = -1;

				auto passed_index = selected_index;

				{
					auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 2; return padding; }());
					ImGui::TabLabels(tabs.size(), tab_names_cstrs.data(), passed_index, nullptr, false, nullptr, ordering.data(), true, true, &closed_tab_index, nullptr);
				}

				/* Read back */

				{
					std::size_t i = 0;

					if (closed_tab_index != -1) {
						for (auto& it : tabs) {
							auto& t = it.second;

							if (i++ == closed_tab_index) {
								close_tab(t);
								break;
							}
						}
					}
					else {
						for (auto& it : tabs) {
							auto& t = it.second;

							for (auto h = 0u; h < ordering.size(); ++h) {
								if (ordering[h] == i) {
									t.horizontal_index = static_cast<std::size_t>(h);
								}
							}

							++i;
						}

						if (passed_index != -1 && passed_index != selected_index) {
							set_tab_by_index(static_cast<std::size_t>(passed_index));
						}
					}
				}
			}
		}
	}

	if (has_tabs()) {
		if (show_summary) {
			auto summary = scoped_window("Summary", &show_summary, ImGuiWindowFlags_AlwaysAutoResize);

			if (has_tabs()) {
				text(typesafe_sprintf("Tick rate: %x/s", get_viewed_cosmos().get_steps_per_second()));
				text(typesafe_sprintf("Total entities: %x/%x",
					get_viewed_cosmos().get_entities_count(),
					get_viewed_cosmos().get_maximum_entities()
				));

				text(
					typesafe_sprintf("World time: %x (%x steps)",
						standard_format_seconds(get_viewed_cosmos().get_total_seconds_passed()),
						get_viewed_cosmos().get_total_steps_passed()
					)
				);

				text(
					typesafe_sprintf(L"Currently viewing: %x",
						get_viewed_character().alive() ? get_viewed_character().get_name() : L"no entity"
					)
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
			
			tab().work.world.for_each_entity_id([&](const entity_id id) {
				const auto handle = tab().work.world[id];
				const auto name = to_string(handle.get_name());

				if (filter.PassFilter(name.c_str())) {
					auto scope = scoped_id(id.indirection_index);

					if (auto node = scoped_tree_node(name.c_str())) {
						if (ImGui::Button("Control")) {
							tab().set_locally_viewed(id);
						}
					}
				}

			});
		}
	}

	if (open_file_dialog.valid() && is_ready(open_file_dialog)) {
		const auto result_path = open_file_dialog.get();

		if (result_path) {
			open_workspace(in_path(*result_path));
		}
	}

	if (save_file_dialog.valid() && is_ready(save_file_dialog)) {
		const auto result_path = save_file_dialog.get();

		if (result_path) {
			save_workspace(in_path(*result_path));
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

static auto get_filters() {
	return std::vector<augs::window::file_dialog_filter> {
		{ "Hypersomnia workspace file (*.wp)", ".wp" },
		{ "Hypersomnia compatibile workspace file (*.lua)", ".lua" },
		{ "All files", ".*" }
	};
}

void editor_setup::open(const augs::window& owner) {
	if (current_popup) {
		return;
	}

	open_file_dialog = std::async(
		std::launch::async,
		[&](){
			return owner.open_file_dialog(get_filters(), "Open workspace");
		}
	);
}

void editor_setup::save(sol::state& lua, const augs::window& owner) {
	if (!has_tabs()) {
		return;
	}

	if (tab().current_path.empty()) {
		save_as(owner);
	}
	else {
		save_workspace({ lua, tab().current_path });
	}
}

void editor_setup::save_as(const augs::window& owner) {
	if (!has_tabs() || current_popup) {
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

void editor_setup::go_to_all() {
	show_go_to_all = true;
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
	try_new_tab([&](editor_tab& t) { 
		t.untitled_index = untitled_index++; 
		return true; 
	});
}

void editor_setup::set_tab_by_index(const std::size_t next_index) {
	std::size_t i = 0;

	for (auto& t : tabs) {
		if (i++ == next_index) {
			set_current_tab(t.second);
		}
	}
}

void editor_setup::set_tab_by_horizontal_index(const std::size_t next_index) {
	const auto found = find_if_in(tabs,
		[next_index](const auto& it) {
			return it.second.horizontal_index == next_index;
		}
	);

	set_current_tab((*found).second);
}

void editor_setup::next_tab() {
	if (has_tabs()) {
		set_tab_by_horizontal_index((current_tab->horizontal_index + 1) % tabs.size());
	}
}

void editor_setup::prev_tab() {
	if (has_tabs()) {
		set_tab_by_horizontal_index([this]() {
			const auto current_index = current_tab->horizontal_index;

			if (current_index == 0) {
				return tabs.size() - 1;
			}
			else {
				return current_index - 1;
			}
		}());
	}
}

void editor_setup::close_tab(editor_tab& tab_to_close) {
	const auto current_horizontal_index = current_tab->horizontal_index;
	const auto closed_horizontal_index = tab_to_close.horizontal_index;
	erase_if(tabs, [&](const auto& it) { return std::addressof(it.second) == std::addressof(tab_to_close); });

	for (auto& it : tabs) {
		if (it.second.horizontal_index > closed_horizontal_index) {
			--it.second.horizontal_index;
		}
	}

	if (has_tabs()) {
		set_tab_by_horizontal_index(std::min(current_horizontal_index, tabs.size() - 1));
	}
	else {
		unset_current_tab();
	}
}

void editor_setup::close_tab() {
	if (current_tab) {
		close_tab(*current_tab);
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
		const auto k = e.key.key;
		
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
					case key::P: go_to_all(); return true;
					case key::N: new_tab(); return true;
					case key::W: close_tab(); return true;
					case key::TAB: next_tab(); return true;
					default: break;
				}
			}

			if (has_shift) {
				switch (k) {
					case key::F5: fill_with_test_scene(lua); return true;
				}
			}

			switch (k) {
				case key::F12: save_as(window); return true;
				case key::ENTER: confirm_modal_popup(); return true;
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
	sol::state& lua
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	if (player_paused) {
		if (e.msg == message::mousemotion) {
			if (common_input_state[key::RMOUSE]) {
				if (has_tabs()) {
					tab().panning -= e.mouse.rel;

					return true;
				}
			}
		}

		if (e.was_pressed(key::HOME)) {
			if (has_tabs()) {
				tab().panning = {};

				return true;
			}
		}

		if (e.was_any_key_pressed()) {
			const auto k = e.key.key;

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
		}
	}

	return false;
}