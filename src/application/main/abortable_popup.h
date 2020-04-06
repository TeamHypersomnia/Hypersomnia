#pragma once

class abortable_popup_state {
	bool is_open = false;
public:

	bool perform(const std::string& title, const std::string& message) {
		using namespace augs::imgui;

		if (auto popup = scoped_modal_popup(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			is_open = true;

			text(message + "\n");
			ImGui::Separator();

			if (ImGui::Button("Abort")) {
				return true;
			}
		}

		return false;
	}
	
	void close() {
		if (is_open) {
			ImGui::CloseCurrentPopup();
			is_open = false;
		}
	}
};
