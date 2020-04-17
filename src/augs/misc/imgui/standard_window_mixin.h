#pragma once
#include "3rdparty/imgui/imgui.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

namespace augs {
	namespace imgui {
		void center_next_window(vec2 size_multiplier, ImGuiCond);
	}
}

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

	void close() {
		show = false;
	}

	template <class... Args>
	auto make_scoped_window(Args&&... args) {
		if (show) {
			if (centered_size_mult != std::nullopt) {
				augs::imgui::center_next_window(*centered_size_mult, ImGuiCond_FirstUseEver);
			}
			else {
				ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiCond_FirstUseEver);
			}
		}

		auto result = augs::imgui::cond_scoped_window(show, title.c_str(), &show, std::forward<Args>(args)...);

		if (result) {
			was_focused = ImGui::IsWindowFocused() || ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		}

		return std::move(result);
	}

	standard_window_mixin(const std::string& title) : title(title) {}

	void acquire_keyboard_once(int offset = 0) {
		if (show && acquire_once) {
			ImGui::SetKeyboardFocusHere(offset);
			acquire_once = false;
		}
	};

	const auto& get_title() const {
		return title;
	}

	bool is_focused() const {
		return show && was_focused;
	}
	
	bool will_acquire_keyboard_once() const {
		return acquire_once;
	}

protected:
	std::optional<vec2> centered_size_mult;

private:
	std::string title;
	bool acquire_once = false;
	bool was_focused = false;
};
