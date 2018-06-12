#include <imgui/imgui.h>
#include "3rdparty/imgui/imgui_internal.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

void property_editor_state::reset() {
	last_tweaked.reset();
	old_description.clear();
}

void property_editor_state::poll_change_of_active_widget() {
	if (last_tweaked && last_tweaked.value() != ImGui::GetActiveID()) {
		last_tweaked.reset();
		old_description = {};
	}
}

bool property_editor_state::tweaked_widget_changed() {
	bool result;

	const auto this_id = ImGui::GetActiveID();

	if (last_tweaked != this_id) {
		result = true;
	}
	else {
		result = false;
	}

	last_tweaked = this_id;
	return result;
}
