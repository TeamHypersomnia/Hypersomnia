#pragma once
#include "augs/templates/enum_introspect.h"

namespace augs {
	namespace imgui {
		template <class T, class... Args>
		auto enum_combo(const std::string& label, T& into, Args&&... args) {
			const auto current_str = format_enum(into);
			const auto current = into;

			if (ImGui::BeginCombo(label.c_str(), current_str.c_str())) {
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

				ImGui::EndCombo();
			}

			return current != into;
		}
	}
}
