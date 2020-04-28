#pragma once
#include "augs/templates/enum_introspect.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/string/format_enum.h"

template <class E, class F>
void do_pretty_tabs(E& active_pane, F custom_name) {
	using namespace augs::imgui;

	{
		auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 4; return padding; }());

		{
			static auto labels = [custom_name]() {
				static augs::enum_array<std::string, E> label_strs;
				augs::enum_array<const char*, E> c_strs;

				augs::for_each_enum_except_bounds([&c_strs, custom_name](const E s) {
					const auto custom = custom_name(s);

					label_strs[s] = custom != std::nullopt ? *custom : format_enum(s);
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

template <class E>
void do_pretty_tabs(E& active_pane) {
	do_pretty_tabs(active_pane, [](const E) -> std::optional<std::string> { return std::nullopt; });
}

