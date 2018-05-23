#pragma once
#include <string>
#include "3rdparty/imgui/imgui.h"
#include "application/setups/editor/editor_command_input.h"

template <class duplicate_command, class delete_command, class I>
void duplicate_delete_buttons(
	const editor_command_input cmd_in,
	const I& id,
	const property_editor_settings& settings,
	const std::string& imgui_id,
	const bool enable_delete
) {
	const auto scoped_style = in_line_button_style();

	if (const auto button_label = "D##" + imgui_id;
		ImGui::Button(button_label.c_str())
	) {
		post_editor_command(cmd_in, duplicate_command(id));
	}

	ImGui::SameLine();

	{
		auto disabled_scope = ::maybe_disabled_cols(settings, !enable_delete);

		if (const auto button_label = "-##" + imgui_id;
			ImGui::Button(button_label.c_str())
		) {
			post_editor_command(cmd_in, delete_command(id));
		}
	}

	ImGui::SameLine();
}

