#include "augs/string/typesafe_sprintf.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/simple_popup.h"

simple_popup simple_popup::sum_all(const std::vector<simple_popup>& popups) {
	simple_popup result;

	// for example: 11 Error(s), 23 Warning(s)

	std::map<std::string, std::size_t> counts;

	for (const auto& p : popups) {
		counts[p.title]++;

		if (p.message.size() > 0) {
			result.message += p.message + "\n\n";
		}

		if (p.details.size() > 0) {
			result.details += p.details + "\n";
		}
	}

	for (const auto& e : counts) {
		result.title += typesafe_sprintf("%x %x(s), ", e.second, e.first);
	}

	if (result.title.size() >= 2) {
		result.title.pop_back();
		result.title.pop_back();
	}

	if (result.message.size() >= 1) {
		result.message.pop_back();
	}

	if (result.details.size() >= 1) {
		result.details.pop_back();
	}

	return result;
}

std::string simple_popup::make_log() const {
	return title + ": " + message;
}

int simple_popup::perform(const std::vector<button>& buttons) {
	using namespace augs::imgui;

	if (auto popup = scoped_modal_popup(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (!warning_notice_above.empty()) {
			text_color(warning_notice_above, yellow);
		}

		text(message);

		if (!warning_notice.empty()) {
			text_color(warning_notice, yellow);
		}

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
