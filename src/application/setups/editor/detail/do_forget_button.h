#pragma once
#include "augs/templates/algorithm_templates.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/detail/pathed_asset_entry.h"

template <class asset_id_type, class A>
bool do_forget_button(
	const editor_command_input cmd_in,
	const pathed_asset_entry<asset_id_type>& path_entry,
	const A& ticked_in_range,
	const bool is_current_ticked,
	const std::string& label
) {
	bool result = false;

	if (!path_entry.used()) {
		using namespace augs::imgui;

		const auto scoped_style = in_line_button_style();

		if (ImGui::Button("F")) {
			bool has_parent = false;

			auto forget = [&](const auto& which) {
				forget_asset_id_command<asset_id_type> cmd;
				cmd.freed_id = which.id;
				cmd.built_description = 
					typesafe_sprintf("Stopped tracking %x", which.get_full_path().to_display())
				;

				cmd.common.has_parent = has_parent;
				post_editor_command(cmd_in, std::move(cmd));

				has_parent = true;
			};

			if (!is_current_ticked) {
				forget(path_entry);
			}
			else {
				::for_each_in(ticked_in_range, forget);
			}

			result = true;
		}

		ImGui::SameLine();

		if (ImGui::IsItemHovered()) {
			if (is_current_ticked && ticked_in_range.size() > 1) {
				text_tooltip("Forget %x %xs", ticked_in_range.size(), label);
			}
			else {
				text_tooltip("Forget %x", path_entry.get_full_path().to_display());
			}
		}
	}

	return result;
}
