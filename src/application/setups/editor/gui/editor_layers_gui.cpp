#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "application/setups/editor/editor_filesystem.h"

#include "application/setups/editor/gui/editor_layers_gui.h"

void editor_layers_gui::perform(const editor_layers_input in) {
	using namespace augs::imgui;

	(void)in;
	
	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	ImGui::Selectable("Test layer");

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("editor_filesystem_gui"))
		{
			if (in.dragged_resource) {
				LOG("Dropped %x on layer", in.dragged_resource->name);

				in.dragged_resource = nullptr;
			}
		}

		ImGui::EndDragDropTarget();
	}
}
