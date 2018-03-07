#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"

void editor_all_entities_gui::open() {
	show = true;
	ImGui::SetWindowFocus("All entities");
}

void editor_all_entities_gui::perform(const editor_command_input in) {
	using namespace augs::imgui;

	auto entities = scoped_window("All entities", &show);
	auto& work = *in.folder.work;
	auto& cosm = in.folder.work->world;

	if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)) {
		ImGui::SetKeyboardFocusHere();
	}

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	cosmic::for_each_entity(cosm, [&](const auto handle) {
		const auto name = to_string(handle.get_name());
		const auto id = handle.get_id();

		if (filter.PassFilter(name.c_str())) {
			auto scope = scoped_id(handle.get_guid());

			int flags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;

			if (work.local_test_subject == id) {
				flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected;
			}

			ImGui::TreeNodeEx(name.c_str(), flags);

			if (ImGui::IsItemClicked()) {

			}
			ImGui::TreePop();
		}

	});
}
