#include "augs/templates/string_templates.h"
#include "augs/misc/imgui_utils.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/log_color.h"

#include "application/setups/editor_setup.h"

#include "generated/introspectors.h"

void editor_setup::set_popup(const popup p) {
	current_popup = p;
}

editor_setup::editor_setup(const editor_settings settings) {
	bool success = false;
	
	try {
		edited_world.load_from_file(augs::path_type(settings.last_workspace_dir) += "cosmos.bin");
		success = true;
	}
	catch (cosmos_loading_error err) {
		set_popup ({
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

void editor_setup::control(
	const cosmic_entropy& entropy
) {

}

void editor_setup::perform_custom_imgui() {
	using namespace augs::imgui;

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