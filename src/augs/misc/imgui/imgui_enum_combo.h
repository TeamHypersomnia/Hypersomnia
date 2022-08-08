#pragma once
#include "augs/templates/enum_introspect.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

namespace augs {
	namespace imgui {
		template <class T, class... Args>
		auto enum_combo(const std::string& label, T& into, const std::string& overridden_label = "", Args&&... args) {
			const auto current_str = overridden_label.empty() ? format_enum(into) : overridden_label;
			const auto current = into;

			const ImGuiID id = ImGui::GetCurrentWindow()->GetID(label.c_str());

			if (auto combo = scoped_combo(label.c_str(), current_str.c_str(), std::forward<Args>(args)...)) {
				for_each_enum_except_bounds([&](const T e) {
					const auto enum_label = format_enum(e);
					bool is_selected = e == current;

					if (ImGui::Selectable(enum_label.c_str(), is_selected)) {
						into = e;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				});

				// To detect that the same combo has been modified in editor
				ImGui::GetCurrentContext()->LastActiveId = id;
			}

			return current != into;
		}
	}
}
