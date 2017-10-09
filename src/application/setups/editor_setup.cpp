#include "augs/templates/string_templates.h"
#include "augs/misc/imgui_utils.h"
#include "augs/misc/imgui_control_wrappers.h"
#include "augs/readwrite/lua_readwrite.h"
#include "augs/filesystem/file.h"
#include "augs/templates/thread_templates.h"
#include "augs/templates/chrono_templates.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/shell.h"

#include "application/config_lua_table.h"
#include "application/setups/editor_setup.h"

#include "generated/introspectors.h"

void editor_setup::set_popup(const editor_popup p) {
	current_popup = p;
}

void editor_tab::set_workspace_path(const path_operation op) {
	current_path = op.path;
	op.recent.add(op.lua, op.path);
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

editor_setup::editor_setup(sol::state& lua) : recent(lua) {
}

editor_setup::editor_setup(sol::state& lua, const augs::path_type& workspace_path) : recent(lua) {
	if (tab().open_workspace({ lua, recent, workspace_path })) {

	}

	if (tab().current_path.empty()) {
		open_untitled_workspace();
	}
}

void editor_setup::control(
	const cosmic_entropy& entropy
) {

}

void editor_setup::customize_for_viewing(config_lua_table& config) const {
	const auto filename = tab().current_path.filename().string();

	config.window.name = "Editor - " + (filename.empty() ? std::string("Untitled") : filename);
	return;
}

void editor_setup::open_workspace(const path_operation op) {
	if (const auto popup = tab().open_workspace({ op.lua, recent, op.path })) {
		set_popup(*popup);
	}
}

void editor_setup::save_workspace(const path_operation op) {
	tab().save_workspace({ op.lua, recent, op.path });
}

void editor_setup::perform_custom_imgui(
	sol::state& lua,
	augs::window& owner,
	const bool game_gui_active
) {
	using namespace augs::imgui;

	auto in_path = [&](const auto& path) {
		return path_operation{ lua, path };
	};

	if (game_gui_active) {
		if (auto main_menu = scoped_main_menu_bar()) {
			if (auto menu = scoped_menu("File")) {
				if (ImGui::MenuItem("New", "CTRL+N")) {}

				if (ImGui::MenuItem("Open", "CTRL+O")) {
					open(owner);
				}

				if (auto menu = scoped_menu("Recent files")) {
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
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Save", "CTRL+S")) {
					save(lua, owner);
				}

				if (ImGui::MenuItem("Save as", "F12")) {
					save_as(owner);
				}
			}
			if (auto menu = scoped_menu("Edit")) {
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+SHIFT+Z", false, false)) {}
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::Separator();

#if	BUILD_TEST_SCENES
				if (ImGui::MenuItem("Fill with test scene")) {
					tab().work.make_test_scene(lua, false);
				}
#else
				if (ImGui::MenuItem("Fill with test scene", nullptr, false, false)) {}
#endif
			}
			if (auto menu = scoped_menu("View")) {
				if (ImGui::MenuItem("Summary")) {
					show_summary = true;
				}
				if (ImGui::MenuItem("Player")) {
					show_player = true;
				}

				ImGui::Separator();
				ImGui::MenuItem("(State)", NULL, false, false);

				if (ImGui::MenuItem("Common")) {
					show_common_state = true;
				}

				if (ImGui::MenuItem("Entities")) {
					show_entities = true;
				}
			}
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

	if (show_summary) {
		auto summary = scoped_window("Summary", &show_summary, ImGuiWindowFlags_AlwaysAutoResize);

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
						tab().work.locally_viewed = id;
					}
				}
			}

		});
	}

	if (current_popup) {
		auto& p = *current_popup;

		if (!ImGui::IsPopupOpen(p.title.c_str())) {
			ImGui::OpenPopup(p.title.c_str());
		}

		if (ImGui::BeginPopupModal(p.title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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

			ImGui::EndPopup();
		}
	}

	if (open_file_dialog.valid() && !is_ready(open_file_dialog)) {

	}
}

void editor_setup::accept_game_gui_events(
	const cosmic_entropy& entropy
) {

}

bool editor_setup::escape_modal_popup() {
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
	open_file_dialog = std::async(
		std::launch::async,
		[&](){
			return owner.open_file_dialog(get_filters(), "Open workspace");
		}
	);
}

void editor_setup::save(sol::state& lua, const augs::window& owner) {
	if (tab().current_path.empty()) {
		save_as(owner);
	}
	else {
		save_workspace({ lua, tab().current_path });
	}
}

void editor_setup::save_as(const augs::window& owner) {
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
	auto& f = player_paused;
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

void editor_setup::next_tab() {

}

void editor_setup::prev_tab() {

}

void editor_setup::close_tab() {

}
