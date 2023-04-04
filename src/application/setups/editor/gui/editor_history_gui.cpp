#include "application/setups/editor/gui/editor_history_gui.h"
#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_setup.h"

#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

void editor_history_gui::perform(const editor_history_gui_input in) {
	using namespace augs::imgui;
	using index_type = editor_history::index_type;

	auto& history = in.history;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	acquire_keyboard_once();

	if (scroll_to_latest_once) {
		ImGui::SetScrollHereY(0.0f);

		scroll_to_latest_once = false;
	}

	const auto& style = ImGui::GetStyle();

	thread_local ImGuiTextFilter filter;
	filter_with_hint(filter, "##HistoryFilter", "Search history...");

	const auto avail = ImGui::GetContentRegionAvail().x;

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, avail - ImGui::CalcTextSize("99999").x);
	text_disabled("Operation");
	ImGui::NextColumn();
	text_disabled("When");
	ImGui::NextColumn();
	ImGui::Separator();

	auto do_history_node = [&](
		const index_type command_index,
		const auto& command
	){
		auto scope = scoped_id(command_index);

		const auto& description = command.describe();
		const augs::date_time& when = command.meta.timestamp;

		const auto how_long_ago = when.how_long_ago_brief();

		const bool passes_filter = filter.PassFilter(description.c_str()) || filter.PassFilter(how_long_ago.c_str());

		if (!passes_filter) {
			return false;
		}

		const auto current_revision = history.get_current_revision();
		const bool is_selected = command_index == current_revision;

		if (is_selected && scroll_to_current_once) {
			ImGui::SetScrollHereY(0.5f);

			scroll_to_current_once = false;
		}

		int colors = 0;

		if (command_index > current_revision) {
			++colors;

			const auto header_hover_color = rgba(style.Colors[ImGuiCol_Button]).multiply_rgb(1 / 1.3f);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, header_hover_color);
		}

		if (history.is_saved_revision(command_index) && !in.setup.is_dirty()) {
			++colors;

			auto saved_color = rgba(0, 200, 0, 255);
			ImGui::PushStyleColor(ImGuiCol_Text, saved_color);
		}
		else if (command_index > current_revision) {
			++colors;

			auto disabled_color = rgba(style.Colors[ImGuiCol_Button]).desaturate();
			ImGui::PushStyleColor(ImGuiCol_Text, disabled_color.operator ImVec4());
		}

		bool result = false;

		if (ImGui::Selectable(description.c_str(), is_selected)) {
			in.setup.seek_to_revision(command_index);
		}

		ImGui::PopStyleColor(colors);

		ImGui::NextColumn();

		text_disabled(how_long_ago /* + " (?)" */);	

		ImGui::NextColumn();

		return result;
	};

	const auto& commands = history.get_commands();

	bool ignore_until_parent = false;

	for (std::size_t i = 0; i < commands.size(); ++i) {
		/*
			We iterate commands in reverse,
			to show the most recent ones on top.
		*/
		const auto n = commands.size() - i - 1;

		std::visit(
			[&](const auto& typed_command) {
				if (ignore_until_parent) {
					if (!typed_command.meta.is_child) {
						ignore_until_parent = false;
					}

					return;
				}

				if (typed_command.meta.is_child) {
					ignore_until_parent = true;
				}

				do_history_node(n, typed_command);
			},
			commands[n]
		);
	}

	{
		struct first_command_dummy {
			editor_command_meta meta;

			auto describe() const {
				return std::string("Opened project file");
			}
		};

		const auto first_command = first_command_dummy {};

		do_history_node(-1, first_command);
	}
}

