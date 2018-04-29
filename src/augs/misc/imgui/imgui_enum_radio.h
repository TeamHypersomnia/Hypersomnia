#pragma once
#include "augs/templates/enum_introspect.h"
#include "augs/string/format_enum.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

namespace augs {
	namespace imgui {
		template <class T, class... Args>
		auto enum_radio(T& into, const bool horizontal = false) {
			const auto current = into;

			bool is_first = true;

			for_each_enum_except_bounds([&](const T e) {
				const auto enum_label = format_enum(e);
				bool is_selected = e == current;

				if (!is_first && horizontal) {
					ImGui::SameLine();
				}

				if (ImGui::RadioButton(enum_label.c_str(), is_selected)) {
					into = e;
				}

				is_first = false;
			});

			return current != into;
		}
	}
}
