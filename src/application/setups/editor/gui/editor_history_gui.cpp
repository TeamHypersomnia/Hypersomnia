#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/gui/editor_history_gui.h"

template <class T>
static bool has_parent(const T& cmd) {
	return std::visit(
		[](const auto& typed_command) {
			return typed_command.common.has_parent;
		},
		cmd
	);
}

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

	auto do_history_node = [&](
		const index_type command_index,
		const std::string& description,
		const augs::date_time& when,
		const bool has_parent,
		const bool is_parent
	){
		const auto how_long_ago = when.how_long_ago();
		const bool passes_filter = filter.PassFilter(description.c_str()) || filter.PassFilter(how_long_ago.c_str());

		if (!passes_filter) {
			return false;
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

		//auto indent = cond_scoped_indent(has_parent);
		(void)has_parent;

		bool result = false;

		if (is_parent) {
			auto flags = static_cast<int>(ImGuiTreeNodeFlags_OpenOnArrow);

			if (is_selected) {
				flags = flags | ImGuiTreeNodeFlags_Selected;
			}

			result = ImGui::TreeNodeEx(description.c_str(), flags);
		}
		else {
			ImGui::Selectable(description.c_str(), is_selected);
		}

		ImGui::PopStyleColor(colors);

		if (ImGui::IsItemClicked()) {
			ImGuiContext& g = *ImGui::GetCurrentContext();

			if (!is_parent || g.ActiveIdClickOffset.x > g.FontSize + ImGui::GetStyle().FramePadding.x * 2) {
				history.seek_to_revision(command_index, in);
			}
		}

		ImGui::NextColumn();

		text_disabled(how_long_ago /* + " (?)" */);	

		if (ImGui::IsItemHovered()) {
			text_tooltip(when.get_readable());
		}

		ImGui::NextColumn();

		return result;
	};

	{
		const auto first_label = in.get_player().has_testing_started() ? "Started playtesting" : "Created project files";
		do_history_node(-1, first_label, in.folder.view.meta.timestamp, false, false);
	}

	const auto& commands = history.get_commands();

	auto do_normal_command = [&](const std::size_t i) {
		std::visit(
			[&](const auto& command) {
				do_history_node(i, command.describe(), command.common.timestamp, command.common.has_parent, false);
			},
			commands[i]
		);
	};

	auto do_parent_command = [&](const std::size_t i) {
		return std::visit(
			[&](const auto& command) {
				return do_history_node(i, command.describe(), command.common.timestamp, command.common.has_parent, true);
			},
			commands[i]
		);
	};

	for (std::size_t i = 0; i < commands.size(); ++i) {
		if (i < commands.size() - 1) {
			if (::has_parent(commands[i + 1])) {
				const bool do_children = do_parent_command(i);

				auto j = i + 1;

				while (j < commands.size() && has_parent(commands[j])) {
					if (do_children) { 
						do_normal_command(j); 
					}

					++j;
				}

				i = j - 1;

				if (do_children) {
					ImGui::TreePop();
				}

				continue;
			}
		}

		do_normal_command(i);
	}
}

