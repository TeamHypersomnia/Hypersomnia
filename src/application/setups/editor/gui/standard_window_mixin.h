#pragma once
#include "3rdparty/imgui/imgui.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

template <class derived>
struct standard_window_mixin {
	// GEN INTROSPECTOR struct standard_window_mixin class derived
	bool show = false;
	// END GEN INTROSPECTOR

	void open() {
		show = true;
		acquire_once = true;
		ImGui::SetWindowFocus(title.c_str());
	}

	template <class... Args>
	auto make_scoped_window(Args&&... args) {
		if (show) {
			ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiCond_FirstUseEver);
		}

		auto result = augs::imgui::cond_scoped_window(show, title.c_str(), &show, std::forward<Args>(args)...);

		if (!show) {
			result.release();
		}

		return result;
	}

	standard_window_mixin(const std::string& title) : title(title) {}

	void acquire_keyboard_once() {
		if (show && acquire_once) {
			ImGui::SetKeyboardFocusHere();
			acquire_once = false;
		}
	};

protected:
	std::string title;
	bool acquire_once = false;
};
