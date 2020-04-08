#pragma once

struct abortable_popup_state {
	bool perform(
		const bool should_popup_be_open, 
		const std::string& title, 
		const std::string& message
	) {
		using namespace augs::imgui;

		if (auto popup = cond_scoped_modal_popup(should_popup_be_open, title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			text(message + "\n");
			ImGui::Separator();

			if (ImGui::Button("Abort")) {
				ImGui::CloseCurrentPopup();
				return true;
			}
		}

		return false;
	}
};
