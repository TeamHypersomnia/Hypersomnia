#pragma once
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "3rdparty/imgui/imgui.h"

template <class C>
void separator_if_unofficials_ended(const C& entry, bool& flag) {
	if (entry.is_official) {
		if (flag) {
			ImGui::Separator();
			flag = false;
		}
	}
	else {
		flag = true;
	}
}

template <class F, class C>
void simple_browse_path_tree(
	path_tree_settings& settings,
	const C& all_paths,
	F path_callback,
	const bool acquire_keyboard,

	const C& disallowed_paths = {},
	const std::string& disallowed_paths_displayed_name = "Busy paths",
	const std::array<std::string, 2> custom_column_names = {}
) {
	using namespace augs::imgui;

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	if (acquire_keyboard) {
		ImGui::SetKeyboardFocusHere();
	}

	auto files_view = scoped_child("Files view", ImVec2(0, 20 * ImGui::GetTextLineHeightWithSpacing()));

	if (settings.linear_view) {
		ImGui::Columns(2);

		if (custom_column_names[0].size() > 0) {
			text_disabled(custom_column_names[0]);
			ImGui::NextColumn();
			text_disabled(custom_column_names[1]);
			ImGui::NextColumn();
		}
		else {
			settings.do_name_location_columns();
		}

		ImGui::Separator();

		bool official_separator = false;

		for (const auto& l : all_paths) {
			const auto prettified = settings.get_prettified(l.get_filename().string());
			const auto displayed_dir = l.get_displayed_directory();

			if (!filter.PassFilter(prettified.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				continue;
			}

			separator_if_unofficials_ended(l.get_full_path(), official_separator);

			path_callback(l, prettified);
			ImGui::NextColumn();
			text_disabled(displayed_dir);
			ImGui::NextColumn();
		}

		if (disallowed_paths.size() > 0) {
			if (all_paths.size() > 0) {
				ImGui::Separator();
			}

			official_separator = false;

			text_disabled(disallowed_paths_displayed_name);
			ImGui::NextColumn();
			text_disabled("Location");
			ImGui::NextColumn();
			ImGui::Separator();

			for (const auto& l : disallowed_paths) {
				const auto prettified = settings.get_prettified(l.get_filename().string());
				const auto displayed_dir = l.get_displayed_directory();

				if (!filter.PassFilter(prettified.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
					continue;
				}

				separator_if_unofficials_ended(l.get_full_path(), official_separator);

				text_disabled(prettified);

				ImGui::NextColumn();
				text_disabled(displayed_dir);
				ImGui::NextColumn();
			}
		}
	}
	else {

	}
}

