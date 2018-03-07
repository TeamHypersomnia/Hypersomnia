#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"

void editor_all_entities_gui::open() {
	show = true;
	ImGui::SetWindowFocus("All entities");
}

void editor_all_entities_gui::perform(const editor_command_input in) {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	auto entities = scoped_window("All entities", &show);
	auto& work = *in.folder.work;
	auto& cosm = in.folder.work->world;

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	ImGui::Columns(2, "mycolumns"); // 4-ways, with border
	ImGui::NextColumn();
	text_disabled("Details");
	ImGui::NextColumn();
	ImGui::Separator();

	cosm.get_solvable().for_each_pool(
		[&](const auto& p){
			using P = decltype(p);
			using pool_type = std::decay_t<P>;

			using E = type_argument_t<typename pool_type::mapped_type>;

			const auto entity_type_label = format_field_name(get_type_name<E>());
			const auto total_entities = p.size();
			const auto total_flavours = cosm.get_flavours<E>().count();

			const auto node = scoped_tree_node_ex(entity_type_label.c_str());

			ImGui::NextColumn();
			text_disabled(typesafe_sprintf("%x Flavours, %x Entities", total_flavours, total_entities));
			ImGui::NextColumn();

			if (node) {
				cosm.for_each_flavour<E>(
					[&](const auto flavour_id, const auto& flavour){
						const auto flavour_label = to_string(flavour.name);

						const auto all_having_flavour = cosm.get_solvable_inferred().name.get_entities_by_flavour_id(flavour_id);

						const auto f_node = scoped_tree_node_ex(flavour_label.c_str());

						ImGui::NextColumn();
						text_disabled(typesafe_sprintf("%x Entities", all_having_flavour.size()));
						ImGui::NextColumn();

						if (f_node) {
							for (const auto& e : all_having_flavour) {
								bool s = false;
								ImGui::Selectable(typesafe_sprintf("%x", cosm[e].get_guid()).c_str(), &s);

								ImGui::NextColumn();
								text(" ");
								ImGui::NextColumn();
							}
						}
					}
				);
			}
		}	
	);
}
