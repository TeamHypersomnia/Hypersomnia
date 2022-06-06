#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/debugger/commands/change_grouping_command.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/gui/debugger_selection_groups_gui.h"
#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/debugger_history.hpp"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/to_bytes.h"

template <class C1, class C2>
bool all_found(const C1& from, const C2& in) {
	for (const auto& elem : from) {
		if (!found_in(in, elem)) {
			return false;
		}
	}
	
	return true;
}

void debugger_selection_groups_gui::perform(const bool has_ctrl, debugger_command_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	acquire_keyboard_once();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	ImGui::Columns(2);
	ImGui::NextColumn();
	ImGui::NextColumn();
	ImGui::Separator();

	const auto& groups = in.folder.commanded->view_ids.selection_groups.get_groups();

	auto& selected = in.folder.commanded->view_ids.selected_entities;
	auto& history = in.get_history();

	for (std::size_t i = 0; i < groups.size(); ++i) {
		const auto& g = groups[i];

		if (g.entries.size() > 0) {
			if (!filter.PassFilter(g.name.c_str())) {
				continue;
			}

			auto name = g.name;

			auto flags = static_cast<int>(ImGuiTreeNodeFlags_OpenOnArrow);

			const bool whole_group_selected = all_found(g.entries, selected);

			if (whole_group_selected) {
				flags = flags | ImGuiTreeNodeFlags_Selected;
			}

			auto node = scoped_tree_node_ex(name.c_str(), flags);

			if (ImGui::IsItemClicked()) {
				if (has_ctrl) {
					for (const auto& e : g.entries) {
						if (whole_group_selected) {
							selected.erase(e);
						}
						else {
							selected.emplace(e);
						}
					}
				}
				else {
					assign_begin_end(selected, g.entries);
				}
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
					augs::assign_bytes(cmd.value_after_change, name);

					history.execute_new(cmd, in);
				}

				ImGui::NextColumn();
				ImGui::NextColumn();
			}
		}
	}
}
