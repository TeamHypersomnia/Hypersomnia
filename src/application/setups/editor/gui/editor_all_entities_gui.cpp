#include "augs/misc/simple_pair.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"

#include "application/setups/editor/property_editor/flavour_properties_editor.h"
#include "application/setups/editor/property_editor/entity_properties_editor.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

void editor_all_entities_gui::open() {
	show = true;
	acquire_once = true;
	ImGui::SetWindowFocus("All entities");
}

void editor_all_entities_gui::interrupt_tweakers() {
	properties_gui.last_active.reset();
	properties_gui.old_description.clear();
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
	next_column_text_disabled("Details");
	ImGui::Separator();

	hovered_guid.unset();

	cosm.get_solvable().for_each_pool(
		[&](const auto& p){
			using P = decltype(p);
			using pool_type = std::decay_t<P>;

			using E = type_argument_t<typename pool_type::mapped_type>;
			using specific_handle = const_typed_entity_handle<E>;

			const auto entity_type_label = format_field_name(get_type_name<E>());
			const auto total_entities = p.size();
			const auto total_flavours = cosm.get_common_significant().get_flavours<E>().count();

			const auto node = scoped_tree_node_ex(entity_type_label);

			next_column_text_disabled(typesafe_sprintf("%x Flavours, %x Entities", total_flavours, total_entities));

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
						) {
							const auto flavour_label = flavour.template get<invariants::name>().name;

							if (!filter.PassFilter(flavour_label.c_str())) {
								return;
							}

							const auto all_having_flavour = cosm.get_solvable_inferred().name.get_entities_by_flavour_id(flavour_id);

							const auto node_label = typesafe_sprintf("%x###%x", flavour_label, flavour_id.raw);
							const auto flavour_node = scoped_tree_node_ex(node_label);

							const auto num_entities_label = typesafe_sprintf("%x Entities", all_having_flavour.size());
							next_column_text_disabled(num_entities_label);

							if (flavour_node) {
								ImGui::Separator();

								{
									auto command_maker = [&flavour_id]() {
										change_flavour_property_command cmd;
										cmd.affected_flavours = { flavour_id };
										return cmd;
									};

									edit_flavour(properties_gui, flavour_id, flavour, command_maker, in);
								}

								ImGui::Separator();

								{
									const auto unified_entities_node = scoped_tree_node_ex(num_entities_label + " (unified)");

									next_column_text();

									if (unified_entities_node) {
										if (all_having_flavour.size() > 0) {
											const auto first_handle = specific_handle(cosm, (*all_having_flavour.begin()).basic());

											auto command_maker = [&all_having_flavour]() {
												change_entity_property_command cmd;
												cmd.affected_entities.assign(all_having_flavour.begin(), all_having_flavour.end());
												return cmd;
											};

											edit_entity(properties_gui, first_handle, command_maker, in);
										}
									}
								}

								const auto entities_node = scoped_tree_node_ex(num_entities_label);

								next_column_text();

								if (entities_node) {
									for (const auto& e : all_having_flavour) {
										auto command_maker = [e]() {
											change_entity_property_command cmd;
											cmd.affected_entities = { e };
											return cmd;
										};

										const auto typed_handle = specific_handle(cosm, e);

										const auto guid = typed_handle.get_guid();
										const auto entity_label = typesafe_sprintf("%x", guid);

										const auto entity_node = scoped_tree_node_ex(entity_label);

										if (ImGui::IsItemHovered()) {
											hovered_guid = guid; 
										}

										next_column_text();

										if (entity_node) {
											edit_entity(properties_gui, typed_handle, command_maker, in);
										}
									}
								}

								ImGui::Separator();
							}
						}
					);

					return changer_callback_result::DONT_REFRESH;
				});
			}
		}	
	);
}
