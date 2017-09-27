#include "augs/templates/string_templates.h"
#include "augs/misc/imgui_utils.h"
#include "augs/filesystem/file.h"
#include "augs/templates/thread_templates.h"
#include "augs/window_framework/platform_utils.h"

#include "application/setups/editor_setup.h"

#include "generated/introspectors.h"

void editor_setup::set_popup(const popup p) {
	current_popup = p;
}

void editor_setup::start_open_file_dialog() {
	open_file_dialog = std::async(
		std::launch::async,
		[](){
			return augs::get_open_file_name(L"Cosmos binary file (*.bin)\0*.bin\0");
		}
	);
}

void editor_setup::open_cosmos(const augs::path_type& cosmos_path) {
	bool success = false;

	try {
		edited_world.load_from_file(cosmos_path);
		current_cosmos_path = cosmos_path;
		success = true;
	}
	catch (cosmos_loading_error err) {
		set_popup({
			"Error",
			"Failed to load the editor workspace.\nA blank default was opened instead.",
			err.what()
		});
	}

	if (const bool load_default = !success) {
		edited_world.set_steps_per_second(60);
		edited_world.reserve_storage_for_entities(100);

		auto origin = edited_world.create_entity("origin_entity");
		origin += components::transform();

		viewed_character_id = origin;
	}
}

editor_setup::editor_setup(const augs::path_type& cosmos_path) {
	open_cosmos(cosmos_path);
}

void editor_setup::control(
	const cosmic_entropy& entropy
) {

}

void editor_setup::perform_custom_imgui() {
	using namespace augs::imgui;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New", "CTRL+N")) {}
			
			if (ImGui::MenuItem("Open", "CTRL+O")) {
				start_open_file_dialog();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {} 
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::Separator();
			
			if (ImGui::MenuItem("Fill with test scene", nullptr, false, BUILD_TEST_SCENES == 1)) {
			
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (open_file_dialog.valid() && is_ready(open_file_dialog)) {
		open_cosmos(open_file_dialog.get());
	}

	ImGui::Begin("Summary");
	ImGui::BeginChild("Cosmos");
	ImGui::Text(typesafe_sprintf("Tick rate: %x/s", edited_world.get_steps_per_second()).c_str());
	
	ImGui::Text(typesafe_sprintf("Total entities: %x/%x", 
		edited_world.get_entities_count(),
		edited_world.get_maximum_entities()
	).c_str());

	ImGui::EndChild();
	ImGui::End();

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
}

void editor_setup::accept_game_gui_events(
	const cosmic_entropy& entropy
) {

}