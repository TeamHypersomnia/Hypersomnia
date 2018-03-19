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
	ImGui::SetWindowFocus(title.c_str());
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

	auto entities = scoped_window(title.c_str(), &show);
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

	auto do_edit_entities = [&](const auto& entity, const auto& entities) {
		using T = std::decay_t<decltype(entities)>;

		{
			auto command_maker = [&]() {
				change_entity_property_command cmd;

				if constexpr(is_container_v<T>) {
					cmd.affected_entities.assign(entities.begin(), entities.end());
				}
				else {
					cmd.affected_entities = { entities };
				}

				return cmd;
			};

			edit_entity(properties_gui, entity, command_maker, in);
		}

		ImGui::Separator();
	};

	auto do_edit_flavours = [&](const auto& flavour, const auto& ids) {
		using T = std::decay_t<decltype(ids)>;

		{
			auto command_maker = [&]() {
				change_flavour_property_command cmd;

				if constexpr(is_container_v<T>) {
					cmd.affected_flavours.assign(ids.begin(), ids.end());
				}
				else {
					cmd.affected_flavours = { ids };
				}

				return cmd;
			};

			edit_flavour(properties_gui, flavour, command_maker, in);
		}

		ImGui::Separator();
	};

	cosm.get_solvable().for_each_pool(
		[&](const auto& p){
			using P = decltype(p);
			using pool_type = std::decay_t<P>;

			using E = type_argument_t<typename pool_type::mapped_type>;
			using specific_handle = const_typed_entity_handle<E>;

			using flavour_id_type = typed_entity_flavour_id<E>;
			using flavour_type = entity_flavour<E>;

			const auto entity_type_label = format_field_name(get_type_name<E>());
			const auto total_entities = p.size();
			const auto total_flavours = cosm.get_common_significant().get_flavours<E>().count();

			const auto node = scoped_tree_node_ex(entity_type_label);

			next_column_text_disabled(typesafe_sprintf("%x Flavours, %x Entities", total_flavours, total_entities));

			if (node) {
				cosm.change_common_significant([&](cosmos_common_significant& common_signi){
					const auto& all_flavours = common_signi.get_flavours<E>();

					if (all_flavours.count() > 1) {
						const auto unified_flavours_node = scoped_tree_node_ex(typesafe_sprintf("%x Flavours (unified)", all_flavours.count()));

						next_column_text();

						if (unified_flavours_node) {
							thread_local std::vector<entity_flavour_id> all_flavour_ids;
							all_flavour_ids.clear();

							all_flavours.for_each([&](
								const flavour_id_type flavour_id,
								const flavour_type& flavour
							) {
								all_flavour_ids.push_back(flavour_id);
							});

							{ 
								const auto first_flavour_id = *all_flavour_ids.begin();
								const auto& first_flavour = all_flavours.get_flavour(first_flavour_id.raw);

								do_edit_flavours(first_flavour, all_flavour_ids);
							}

							if (total_entities > 0) {
								const auto unified_entities_node = scoped_tree_node_ex(typesafe_sprintf("%x Entities of %x Flavours (unified)", total_entities, all_flavour_ids.size()));

								next_column_text();

								if (unified_entities_node) {
									thread_local std::vector<entity_id> all_having_flavours;	
									all_having_flavours.clear();

									/*
										This could be done by just iterating over the pool and gathering all ids.
										This however will be better solution if we filter per flavours.
									*/

									for (const auto id : all_flavour_ids) {
										concatenate(
											all_having_flavours,
											cosm.get_solvable_inferred().name.get_entities_by_flavour_id(id)
										);
									}

									ensure(total_entities == all_having_flavours.size());

									const auto first_handle = specific_handle(cosm, (*all_having_flavours.begin()).basic());
									do_edit_entities(first_handle, all_having_flavours);
								}
							}
						}
					}

					all_flavours.for_each(
						[&](const flavour_id_type flavour_id, const flavour_type& flavour) {
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

								do_edit_flavours(flavour, flavour_id);

								{
									const auto unified_entities_node = scoped_tree_node_ex(num_entities_label + " (unified)");

									next_column_text();

									if (unified_entities_node) {
										if (all_having_flavour.size() > 0) {
											const auto first_handle = specific_handle(cosm, (*all_having_flavour.begin()).basic());
											do_edit_entities(first_handle, all_having_flavour);
										}
									}
								}

								const auto entities_node = scoped_tree_node_ex(num_entities_label);

								next_column_text();

								if (entities_node) {
									for (const auto& e : all_having_flavour) {
										const auto typed_handle = specific_handle(cosm, e);

										const auto guid = typed_handle.get_guid();
										const auto entity_label = typesafe_sprintf("%x", guid);

										const auto entity_node = scoped_tree_node_ex(entity_label);

										if (ImGui::IsItemHovered()) {
											hovered_guid = guid; 
										}

										next_column_text();

										if (entity_node) {
											do_edit_entities(typed_handle, e);
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
