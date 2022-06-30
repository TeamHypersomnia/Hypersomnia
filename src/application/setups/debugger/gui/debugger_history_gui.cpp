#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/debugger_player.h"
#include "application/setups/debugger/gui/debugger_history_gui.h"

template <class T>
static bool has_parent(const T& cmd) {
	return std::visit(
		[](const auto& typed_command) {
			return typed_command.common.has_parent;
		},
		cmd
	);
}

void debugger_history_gui::perform(const debugger_command_input in) {
	using namespace augs::imgui;
	using index_type = debugger_history::index_type;

	auto& history = in.get_history();

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	const bool playtesting = in.get_player().has_testing_started();

	acquire_keyboard_once();

	const auto& style = ImGui::GetStyle();

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
		const auto& command,
		const bool is_parent
	){
		auto scope = scoped_id(command_index);

		const auto& description = command.describe();
		const augs::date_time& when = command.common.timestamp;
		const bool has_parent = command.common.has_parent;

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

		if (!playtesting && history.is_saved_revision(command_index)) {
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
			auto cmd_tooltip = when.get_readable();
			cmd_tooltip += typesafe_sprintf("\nEditor step: %x", command.common.when_happened);

			text_tooltip(cmd_tooltip);
		}

		ImGui::NextColumn();

		return result;
	};

	{
		struct first_command_dummy {
			bool playtesting;

			struct common_type {
				bool has_parent = false;
				augs::date_time timestamp;
				decltype(debugger_command_common::when_happened) when_happened = 0;
			} common;

			auto describe() const {
				return std::string(playtesting ? "Started playtesting" : "Created project files");
			}
		};

		const auto first_command = first_command_dummy {
			playtesting,
			{ false, history.when_created, 0 }
		};

		do_history_node(-1, first_command, false);
	}

	const auto& commands = history.get_commands();

	auto do_normal_command = [&](const std::size_t i) {
		std::visit(
			[&](const auto& typed_command) {
				do_history_node(i, typed_command, false);
			},
			commands[i]
		);
	};

	auto do_parent_command = [&](const std::size_t i) {
		return std::visit(
			[&](const auto& typed_command) {
				return do_history_node(i, typed_command, true);
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

