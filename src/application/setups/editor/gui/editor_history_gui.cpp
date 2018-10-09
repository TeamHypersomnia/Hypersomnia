#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/gui/editor_history_gui.h"

void editor_history_gui::perform(const editor_command_input in) {
	using namespace augs::imgui;
	using index_type = editor_history::index_type;

	auto& history = in.get_history();

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	acquire_keyboard_once();

	const auto& style = ImGui::GetStyle();

	if (acquire_once) {
		ImGui::SetKeyboardFocusHere();
		acquire_once = false;
	}

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	ImGui::Columns(2);
	text_disabled("Operation");
	ImGui::NextColumn();
	text_disabled("When");
	ImGui::NextColumn();
	ImGui::Separator();

	const auto playtest_start_revision = [&]() -> std::optional<index_type> {
		const auto& p = in.folder.player;

		if (p.has_testing_started()) {
			return p.get_revision_when_started_testing();
		}

		return std::nullopt;
	}();

	auto do_history_node = [&](
		const index_type command_index,
		const std::string& description,
		const augs::date_time& when,
		const bool has_parent
	){
		const auto how_long_ago = when.how_long_ago();
		const bool passes_filter = filter.PassFilter(description.c_str()) || filter.PassFilter(how_long_ago.c_str());

		if (!passes_filter) {
			return;
		}

		const auto current_revision = history.get_current_revision();
		const bool is_selected = command_index == current_revision;

		int colors = 0;

		if (command_index > current_revision) {
			++colors;

			const auto header_hover_color = rgba(style.Colors[ImGuiCol_Button]).multiply_rgb(1 / 1.3f);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, header_hover_color);
		}

		if (history.is_revision_saved(command_index)) {
			++colors;

			auto saved_color = rgba(0, 200, 0, 255);
			ImGui::PushStyleColor(ImGuiCol_Text, saved_color);
		}
		else if (command_index > current_revision) {
			++colors;

			auto disabled_color = rgba(style.Colors[ImGuiCol_Button]).desaturate();
			ImGui::PushStyleColor(ImGuiCol_Text, disabled_color.operator ImVec4());
		}

		auto indent = cond_scoped_indent(has_parent);

		if (playtest_start_revision && command_index < *playtest_start_revision) {
			text_disabled(description);
		}
		else {
			ImGui::Selectable(description.c_str(), is_selected);
		}

		ImGui::PopStyleColor(colors);

		if (ImGui::IsItemClicked()) {
			history.seek_to_revision(command_index, in);
		}

		if (playtest_start_revision == command_index) {
			//ImGui::PushStyleColor(ImGuiCol_HeaderHovered, red);
			//ImGui::Selectable("(Playtesting started)", false);
			//ImGui::PopStyleColor();
			text_disabled("(Playtesting started)");
		}

		ImGui::NextColumn();

		text_disabled(how_long_ago /* + " (?)" */);	

		if (ImGui::IsItemHovered()) {
			text_tooltip(when.get_readable());
		}

		ImGui::NextColumn();
	};

	do_history_node(-1, "Created project files", in.folder.view.meta.timestamp, false);

	const auto& commands = history.get_commands();

	for (std::size_t i = 0; i < commands.size(); ++i) {
		const auto& c = commands[i];

		std::visit(
			[&](const auto& command) {
				do_history_node(i, command.describe(), command.common.timestamp, command.common.has_parent);
			},
			c
		);
	}
}

