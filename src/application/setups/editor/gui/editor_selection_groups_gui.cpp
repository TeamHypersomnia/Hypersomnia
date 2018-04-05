#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/commands/change_grouping_command.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_selection_groups_gui.h"
#include "application/setups/editor/editor_command_input.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"

void editor_selection_groups_gui::open() {
	show = true;
	ImGui::SetWindowFocus("Selection groups");
}

template <class C1, class C2>
bool all_found(const C1& from, const C2& in) {
	for (const auto& elem : from) {
		if (!found_in(in, elem)) {
			return false;
		}
	}
	
	return true;
}

void editor_selection_groups_gui::perform(editor_command_input in) {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiCond_FirstUseEver);

	auto window = scoped_window("Selection groups", &show);

	ImGui::Columns(2);
	ImGui::NextColumn();
	ImGui::NextColumn();
	ImGui::Separator();

	auto& work = *in.folder.work;

	const auto& groups = in.folder.view.selection_groups.get_groups();

	auto& selected = in.folder.view.selected_entities;
	auto& history = in.folder.history;

	for (std::size_t i = 0; i < groups.size(); ++i) {
		const auto& g = groups[i];
		const auto& name = g.name;

		if (g.entries.size() > 0) {
			auto name = g.name;

			auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

			if (all_found(g.entries, selected)) {
				flags = flags | ImGuiTreeNodeFlags_Selected;
			}

			auto node = scoped_tree_node_ex(name.c_str(), flags);

			if (ImGui::IsItemClicked()) {
				assign_begin_end(selected, g.entries);
			}

			ImGui::NextColumn();

			/* { */
			/* 	const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1)); */

			/* 	const auto id = scoped_id(static_cast<int>(i)); */

			/* 	if (ImGui::Button("Un")) { */

			/* 	} */

			/* 	ImGui::SameLine(); */
			/* } */

			text_disabled(typesafe_sprintf("%x Entities", g.entries.size()));

			ImGui::SameLine();
			ImGui::NextColumn();

			if (node) {
				if (input_text<256>("Name", name, ImGuiInputTextFlags_EnterReturnsTrue)) {
					change_group_property_command cmd;

					cmd.built_description = typesafe_sprintf("Renamed %x to %x", g.name, name);
					cmd.group_index = static_cast<unsigned>(i);
					cmd.value_after_change = augs::to_bytes(name);

					history.execute_new(cmd, in);
				}

				ImGui::NextColumn();
				ImGui::NextColumn();
			}
		}
	}
}
