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

	auto window = scoped_window("History", &show, ImGuiWindowFlags_AlwaysAutoResize);

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

		if (command_index == f.history.get_current_revision()) {
			flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
		}

		scoped_tree_node_ex(description.c_str(), flags);

		if (ImGui::IsItemClicked()) {
			f.history.seek_to_revision(command_index, f);
		}

		ImGui::SameLine(200.f);

		text_disabled(how_long_ago + " (?)");	

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

