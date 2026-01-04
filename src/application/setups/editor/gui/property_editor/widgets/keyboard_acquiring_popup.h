#pragma once
#include <optional>
#include "3rdparty/imgui/imgui.h"

struct keyboard_acquiring_popup {
	int acquire_keyboard_times = 2;
	std::optional<ImGuiID> currently_opened;

	bool check_opened_first_time() {
		const auto& g = *ImGui::GetCurrentContext();

		if (currently_opened != g.OpenPopupStack[g.OpenPopupStack.Size - 1].PopupId) {
			currently_opened = g.OpenPopupStack[g.OpenPopupStack.Size - 1].PopupId;
			acquire_keyboard_times = 2;
			return true;
		}

		return false;
	}

	bool pop_acquire_keyboard() {
		if (acquire_keyboard_times > 0) {
			--acquire_keyboard_times;
			return true;
		}

		return false;
	}

	void mark_not_opened() {
		const auto flags = 0;

		if (currently_opened && !ImGui::IsPopupOpen(*currently_opened, flags)) {
			currently_opened = std::nullopt;
		}
	}

	template <class... Args>
	auto standard_combo_facade(Args&&... combo_args) {
		using namespace augs::imgui;

		auto combo = scoped_combo(std::forward<Args>(combo_args)..., ImGuiComboFlags_HeightLargest);

		if (combo) {
			check_opened_first_time();
		}
		else {
			mark_not_opened();
		}

		return combo;
	}
};
