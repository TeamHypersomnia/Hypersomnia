#pragma once
#include "augs/templates/enum_introspect.h"

template <class E>
void do_pretty_tabs(E& active_pane) {
	using namespace augs::imgui;

	{
		auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 4; return padding; }());

		{
			static auto labels = []() {
				static augs::enum_array<std::string, E> label_strs;
				augs::enum_array<const char*, E> c_strs;

				augs::for_each_enum_except_bounds([&c_strs](const E s) {
					label_strs[s] = format_enum(s);
					c_strs[s] = label_strs[s].c_str();
				});

				return c_strs;
			}();

			auto index = static_cast<int>(active_pane);
			ImGui::TabLabels(labels.data(), static_cast<int>(labels.size()), index, nullptr);
			active_pane = static_cast<E>(index);
		}
	}

	{
		auto scope = scoped_style_color(ImGuiCol_Separator, ImGui::GetStyle().Colors[ImGuiCol_Button]);
		ImGui::Separator();
	}
}
