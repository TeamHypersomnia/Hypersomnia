#include "augs/templates/string_templates.h"
#include "augs/misc/imgui_utils.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/log_color.h"

#include "application/setups/editor_setup.h"

#include "generated/introspectors.h"

editor_setup::editor_setup(const editor_settings settings) {
	edited_world.set_steps_per_second(60);
	edited_world.reserve_storage_for_entities(100);

	auto origin = edited_world.create_entity("origin_entity");
	origin += components::transform();

	viewed_character_id = origin;
}

void editor_setup::control(
	const cosmic_entropy& entropy
) {

}

void editor_setup::perform_custom_imgui() {
	ImGui::Begin("Summary");
	ImGui::BeginChild("Cosmos");
	ImGui::Text(typesafe_sprintf("Tick rate: %x/s", edited_world.get_steps_per_second()).c_str());
	
	ImGui::Text(typesafe_sprintf("Total entities: %x/%x", 
		edited_world.get_entities_count(),
		edited_world.get_maximum_entities()
	).c_str());

	ImGui::EndChild();
	ImGui::End();
}

void editor_setup::accept_game_gui_events(
	const cosmic_entropy& entropy
) {

}