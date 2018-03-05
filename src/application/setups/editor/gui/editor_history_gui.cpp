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


	auto window = scoped_window("History", &show, ImGuiWindowFlags_AlwaysAutoResize);

	const auto& style = GetStyle();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	{
		const auto d = "Created project files";
		const auto when = f.view.meta.timestamp.how_long_ago();

		if (filter.PassFilter(d) || filter.PassFilter(when.c_str())) {
			int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

			if (-1 == f.history.get_current_revision()) {
				flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
			}

			ImGui::TreeNodeEx(d, flags);

			if (ImGui::IsItemClicked()) {
				f.history.seek_to_revision(-1, f);
			}

			ImGui::SameLine(200.f);

			{
				auto scope = scoped_style_color(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
				text(f.view.meta.timestamp.how_long_ago());	
			}

			ImGui::TreePop();
		}
	}

	const auto& commands = f.history.get_commands();

	for (std::size_t i = 0; i < commands.size(); ++i) {
		const auto& c = commands[i];

		std::visit(
			[&](const auto& command) {
				const auto d = command.describe();
				const auto when = command.timestamp.how_long_ago();

				if (filter.PassFilter(d.c_str()) || filter.PassFilter(when.c_str())) {
					int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

					if (i == f.history.get_current_revision()) {
						flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
					}

					ImGui::TreeNodeEx(d.c_str(), flags);

					if (ImGui::IsItemClicked()) {
						f.history.seek_to_revision(i, f);
					}

					ImGui::SameLine(200.f);

					{
						auto scope = scoped_style_color(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
						text(when);	
					}

					ImGui::TreePop();
				}
			},
			c
		);
	}
}

