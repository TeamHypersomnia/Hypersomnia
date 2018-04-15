#pragma once
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/addons/imguitabwindow/imguitabwindow.h"

#include "application/setups/editor/editor_significant.h"

template <class C, class S, class T>
void perform_editor_tab_gui(
	C close_callback,
	S set_current_callback,
	editor_significant& signi,
	const T height
) {
	using namespace augs::imgui;

	if (const auto tab_menu = scoped_tab_menu_bar(height)) {
		{
			using namespace ImGui;

			const auto& in_style = GetStyle();
			auto& out_style = TabLabelStyle::style;

			using col = TabLabelStyle::Colors;

			out_style.rounding = 0;
			out_style.closeButtonRounding = 0;
			out_style.closeButtonBorderWidth = 0;
			out_style.colors[col::Col_TabLabel] = 0;
			out_style.colors[col::Col_TabLabelHovered] = GetColorU32(in_style.Colors[ImGuiCol_ButtonHovered]);
			out_style.colors[col::Col_TabLabelActive] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
			out_style.colors[col::Col_TabLabelText] = GetColorU32(in_style.Colors[ImGuiCol_Text]);
			out_style.colors[col::Col_TabLabelSelected] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
			out_style.colors[col::Col_TabLabelSelectedHovered] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
			out_style.colors[col::Col_TabLabelSelectedActive] = GetColorU32(in_style.Colors[ImGuiCol_ButtonActive]);
			out_style.colors[col::Col_TabLabelSelectedText] = GetColorU32(in_style.Colors[ImGuiCol_Text]);
		}

		// Tab algorithm i/o

		auto selected_index = static_cast<int>(signi.current_index);
		int closed_tab_index{ -1 };

		auto ordering = [&]() {
			std::vector<int> out;
			out.resize(signi.folders.size());

			for (int i = 0; i < out.size(); ++i) {
				out[i] = i;
			}

			return out;
		}();

		{
			const auto tab_names = [&]() {
				std::vector<std::string> out;
				out.reserve(signi.folders.size());

				for (const auto& it : signi.folders) {
					auto p = it.get_display_path();

					if (it.at_unsaved_revision()) {
						p += " *";
					}

					out.push_back(p);
				}

				return out;
			}();

			auto tab_names_cstrs = [&]() {
				std::vector<const char*> out;
				out.reserve(signi.folders.size());

				for (const auto& it : tab_names) {
					out.push_back(it.c_str());
				}

				return out;
			}();

			auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 2; return padding; }());
			ImGui::TabLabels(static_cast<int>(signi.folders.size()), tab_names_cstrs.data(), selected_index, nullptr, false, nullptr, ordering.data(), true, true, &closed_tab_index, nullptr);
		}

		/* Read back */

		{
			if (closed_tab_index != -1) {
				close_callback(static_cast<folder_index>(closed_tab_index));
			}
			else {
				bool changed_order = false;

				for (std::size_t i = 0; i < ordering.size(); ++i) {
					if (ordering[i] != i) {
						changed_order = true;
						break;
					}
				}

				auto index_to_set = static_cast<folder_index>(selected_index);

				if (changed_order) {
					decltype(signi.folders) new_tabs;
					new_tabs.reserve(signi.folders.size());

					for (const auto o : ordering) {
						if (o == selected_index) {
							index_to_set = static_cast<folder_index>(new_tabs.size());
						}

						new_tabs.push_back(std::move(signi.folders[o]));
					}

					signi.folders = std::move(new_tabs);
				}

				set_current_callback(index_to_set);
			}
		}
	}
}
