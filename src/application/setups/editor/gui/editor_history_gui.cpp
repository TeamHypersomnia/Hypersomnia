#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_history_gui.h"

void editor_history_gui::perform(editor_folder& f) {
	if (!show) {
		return;
	}

	using namespace ImGui;
	using namespace augs::imgui;
	using index_type = editor_history::index_type;

	auto window = scoped_window("History", &show);

	const auto& style = GetStyle();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	auto do_history_node = [&](
		const index_type command_index,
		const std::string& description,
		const augs::date_time& when	
	){
		const auto how_long_ago = when.how_long_ago();
		const bool passes_filter = filter.PassFilter(description.c_str()) || filter.PassFilter(how_long_ago.c_str());

		if (!passes_filter) {
			return;
		}

		int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

		const auto current_revision = f.history.get_current_revision();

		if (command_index == current_revision) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (command_index > current_revision) {
			auto disabled_color = style.Colors[ImGuiCol_CloseButton];
			auto header_hover_color = style.Colors[ImGuiCol_CloseButton];

			header_hover_color.x /= 1.3;
			header_hover_color.y /= 1.3;
			header_hover_color.z /= 1.3;

			ImGui::PushStyleColor(ImGuiCol_Text, disabled_color);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, header_hover_color);
		}

		scoped_tree_node_ex(description.c_str(), flags);

		if (command_index > current_revision) {
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		if (ImGui::IsItemClicked()) {
			f.history.seek_to_revision(command_index, f);
		}

		ImGui::SameLine(200.f);

		text_disabled(how_long_ago /* + " (?)" */);	

		if (ImGui::IsItemHovered()) {
			text_tooltip(when.get_readable());
		}
	};

	do_history_node(-1, "Created project files", f.view.meta.timestamp);

	const auto& commands = f.history.get_commands();

	for (std::size_t i = 0; i < commands.size(); ++i) {
		const auto& c = commands[i];

		std::visit(
			[&](const auto& command) {
				do_history_node(i, command.describe(), command.timestamp);
			},
			c
		);
	}
}

