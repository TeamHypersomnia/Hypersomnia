#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/editor_popup.h"

bool editor_popup::perform() {
	using namespace augs::imgui;

	if (!ImGui::IsPopupOpen(title.c_str())) {
		ImGui::OpenPopup(title.c_str());
	}

	if (auto popup = scoped_modal_popup(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		text(message);

		{
			if (details.size() > 0) {
				auto& f = details_expanded;

				if (ImGui::Button(f ? "Hide details" : "Show details")) {
					f = !f;
				}

				if (f) {
					text(details);
				}
			}
		}

		if (ImGui::Button("OK", ImVec2(120, 0))) { 
			ImGui::CloseCurrentPopup();
			return true;
		}
	}

	return false;
}
