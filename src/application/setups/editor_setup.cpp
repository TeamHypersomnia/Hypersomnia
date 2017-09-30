#include "augs/templates/string_templates.h"
#include "augs/misc/imgui_utils.h"
#include "augs/filesystem/file.h"
#include "augs/templates/thread_templates.h"
#include "augs/window_framework/platform_utils.h"

#include "application/config_lua_table.h"
#include "application/setups/editor_setup.h"

#include "generated/introspectors.h"

void editor_setup::set_popup(const popup p) {
	current_popup = p;
}

void editor_setup::start_open_file_dialog() {
	open_file_dialog = std::async(
		std::launch::async,
		[](){
			return augs::get_open_file_name(L"Cosmos binary file (*.bin)\0*.BIN\0");
		}
	);
}

void editor_setup::open_workspace(const augs::path_type& workspace_path) {
	current_workspace_path = workspace_path;
}

void editor_setup::open_blank_workspace() {
	work.world = cosmos::empty;
	work.world.reserve_storage_for_entities(100);

	auto origin = work.world.create_entity("origin_entity");
	origin += components::transform();

	viewed_character_id = origin;

	current_workspace_path = "untitled_cosmos.bin";
}

editor_setup::editor_setup(const augs::path_type& workspace_path) {
	try {
		open_workspace(workspace_path);
	}
	catch (cosmos_loading_error err) {
		set_popup({
			"Error",
			"Failed to load the editor workspace.\nA blank default was opened instead.",
			err.what()
		});

		open_blank_workspace();
	}
}

void editor_setup::control(
	const cosmic_entropy& entropy
) {

}

void editor_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Editor - " + current_workspace_path.string();
	return;
}

void editor_setup::perform_custom_imgui(
	sol::state& lua
) {
	using namespace augs::imgui;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New", "CTRL+N")) {}

			if (ImGui::MenuItem("Open", "CTRL+O")) {
				start_open_file_dialog();
			}

			if (ImGui::MenuItem("Save", "CTRL+S")) {

			}

			if (ImGui::MenuItem("Save as", "F12")) {

			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+SHIFT+Z", false, false)) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::Separator();

#if BUILD_TEST_SCENES
			if (ImGui::MenuItem("Fill with test scene")) {
				work.make_test_scene(lua, false);
				viewed_character_id = work.world.get_entity_by_name(L"player0");
			}
#else
			if (ImGui::MenuItem("Fill with test scene", nullptr, false, false)) {}
#endif

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Summary")) {
				show_summary = true;
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (open_file_dialog.valid() && is_ready(open_file_dialog)) {
		const auto result_path = open_file_dialog.get();
		
		if (result_path) {
			try {
				open_workspace(*result_path);
			}
			catch (cosmos_loading_error err) {
				set_popup({
					"Error",
					"Failed to load the file specified.\nA blank default was opened instead.",
					err.what()
				});

				open_blank_workspace();
			}
		}
	}

	if (show_summary) {
		{
			const auto screen_size = vec2(ImGui::GetIO().DisplaySize);
			const auto initial_settings_size = screen_size / 2;

			ImGui::SetNextWindowPos(screen_size / 2 - initial_settings_size / 2, ImGuiSetCond_FirstUseEver);
			ImGui::SetNextWindowSize(initial_settings_size, ImGuiSetCond_FirstUseEver);
		}

		ImGui::Begin("Summary", &show_summary);
		ImGui::BeginChild("Cosmos");
		ImGui::Text(typesafe_sprintf("Tick rate: %x/s", get_viewed_cosmos().get_steps_per_second()).c_str());
		
		ImGui::Text(typesafe_sprintf("Total entities: %x/%x", 
			get_viewed_cosmos().get_entities_count(),
			get_viewed_cosmos().get_maximum_entities()
		).c_str());

		ImGui::EndChild();
		ImGui::End();
	}

	if (current_popup) {
		auto& p = *current_popup;

		if (!ImGui::IsPopupOpen(p.title.c_str())) {
			ImGui::OpenPopup(p.title.c_str());
		}

		if (ImGui::BeginPopupModal(p.title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text(p.message.c_str());

			{
				auto& f = p.details_expanded;

				if (ImGui::Button(f ? "Hide details" : "Show details")) {
					f = !f;
				}

				if (f) {
					ImGui::Text(p.details.c_str());
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

void editor_setup::handle_open_shortcut() {
	start_open_file_dialog();
}

void editor_setup::handle_save_shortcut() {

}

void editor_setup::handle_save_as_shortcut() {

}

void editor_setup::handle_undo_shortcut() {

}

void editor_setup::handle_redo_shortcut() {

}

void editor_setup::handle_copy_shortcut() {

}

void editor_setup::handle_cut_shortcut() {

}

void editor_setup::handle_paste_shortcut() {

}
