#include "augs/misc/simple_pair.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"

#include "application/setups/editor/gui/editor_properties_gui.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

void editor_all_entities_gui::open() {
	show = true;
	acquire_once = true;
	ImGui::SetWindowFocus("All entities");
}

void editor_all_entities_gui::perform(const editor_command_input in) {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	auto entities = scoped_window("All entities", &show);
	auto& work = *in.folder.work;
	auto& cosm = work.world;

	if (acquire_once) {
		ImGui::SetKeyboardFocusHere();
		acquire_once = false;
	}

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	ImGui::Columns(2); // 4-ways, with border
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
			const auto total_flavours = cosm.get_common_significant().get_flavours<E>().count();

			ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);

			const auto node = scoped_tree_node_ex(entity_type_label.c_str());

			ImGui::NextColumn();
			text_disabled(typesafe_sprintf("%x Flavours, %x Entities", total_flavours, total_entities));
			ImGui::NextColumn();

			if (node) {
				cosm.change_common_significant([&](cosmos_common_significant& common_signi){
					common_signi.get_flavours<E>().for_each(
						[&](
							const auto flavour_id,
							/* 
								Note: we accept flavour as const, 
								because ImGUI itself should only see the immutable reference.

							   	Is the job of the change_flavour_property_command to actually alter flavour state.
							*/
						   	const auto& flavour
						){
							const auto flavour_label = flavour.template get<invariants::name>().name;

							if (!filter.PassFilter(flavour_label.c_str())) {
								return;
							}

							const auto all_having_flavour = cosm.get_solvable_inferred().name.get_entities_by_flavour_id(flavour_id);

							const auto node_label = typesafe_sprintf("%x###%x", flavour_label, flavour_id.raw);
							const auto f_node = scoped_tree_node_ex(node_label.c_str());

							next_column_text_disabled(typesafe_sprintf("%x Entities", all_having_flavour.size()));

							if (f_node) {
								ImGui::Separator();
								edit_flavour(properties_gui, flavour_id, flavour, in);
								ImGui::Separator();

								for (const auto& e : all_having_flavour) {
									bool s = false;
									ImGui::Selectable(typesafe_sprintf("%x", cosm[e].get_guid()).c_str(), &s);

									next_column_text();
								}
							}
						}
					);

					return changer_callback_result::DONT_REFRESH;
				});
			}
		}	
	);
}
