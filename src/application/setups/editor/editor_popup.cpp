#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/editor_popup.h"

int editor_popup::perform(const std::vector<button>& buttons) {
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

		if (buttons.empty()) {
			if (ImGui::Button("OK", ImVec2(120, 0))) { 
				ImGui::CloseCurrentPopup();
				return 1;
			}
		}
		else {
			for (const auto& b : buttons) {
				const bool custom_cols = b.col != rgba::zero;

				const auto cols = std::make_tuple(
					cond_scoped_style_color(custom_cols, ImGuiCol_Button, b.col),
					cond_scoped_style_color(custom_cols, ImGuiCol_ButtonHovered, b.col + b.increment), 
					cond_scoped_style_color(custom_cols, ImGuiCol_ButtonActive, b.col + b.increment + b.increment)
				);

				if (ImGui::Button(b.label.c_str(), ImVec2(120, 0))) { 
					ImGui::CloseCurrentPopup();
					return index_in(buttons, b) + 1;
				}

				ImGui::SameLine();
			}
		}
	}

	return 0;
}
