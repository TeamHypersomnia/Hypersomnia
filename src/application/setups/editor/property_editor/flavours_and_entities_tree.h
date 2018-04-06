#pragma once
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "game/organization/for_each_entity_type.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/flavour_properties_editor.h"
#include "application/setups/editor/property_editor/entity_properties_editor.h"

#include "application/setups/editor/property_editor/flavours_and_entities_tree_structs.h"

template <class E>
void do_edit_entities_gui(
	const property_editor_input& prop_in,
	editor_command_input in,
	const const_typed_entity_handle<E>& entity,
	const affected_entities_type& ids
) {
	change_entity_property_command cmd;
	cmd.type_id.set<E>();
	cmd.affected_entities = ids;

	edit_entity(prop_in, entity, cmd, in);

	ImGui::Separator();
}

template <class E>
void do_edit_flavours_gui(
	const property_editor_input& prop_in,
	editor_command_input in,
	const entity_flavour<E>& flavour,
	const affected_flavours_type& ids
) {
	change_flavour_property_command cmd;
	cmd.type_id.set<E>();
	cmd.affected_flavours = ids;

	edit_flavour(prop_in, flavour, cmd, in);

	ImGui::Separator();
}

template <class F>
auto flavours_and_entities_tree(
	const flavours_and_entities_tree_input& fae_in,
	editor_command_input in,
	F&& flavours_and_entities_provider
) {
	using namespace augs::imgui;

	auto prop_in = fae_in.prop_in;

	flavours_and_entities_tree_filter filter;

	auto& work = *in.folder.work;
	auto& cosm = work.world;

	auto do_edit_entities = [&](const auto& entity, const auto& ids) {
		thread_local affected_entities_type input;

		if constexpr(is_container_v<std::decay_t<decltype(ids)>>) {
			input.assign(ids.begin(), ids.end());
		}
		else {
			input = { ids };
		}

		do_edit_entities_gui(prop_in, in, entity, input);	
	};

	auto do_edit_flavours = [&](const auto& flavour, const auto& ids) {
		thread_local affected_flavours_type input;

		if constexpr(is_container_v<std::decay_t<decltype(ids)>>) {
			input.assign(ids.begin(), ids.end());
		}
		else {
			input = { ids };
		}

		do_edit_flavours_gui(prop_in, in, flavour, input);	
	};

	auto& provider = flavours_and_entities_provider;

	std::size_t total_types = 0;

	for_each_entity_type([&](auto e){
		using E = decltype(e);

		if (provider.template num_entities_of_type<E>() > 0) {
			++total_types;
		}
	});

	cosm.get_solvable().for_each_pool(
		[&](const auto& p){
			using P = decltype(p);
			using pool_type = std::decay_t<P>;

			using Solvable = typename pool_type::mapped_type;
			using E = typename Solvable::used_entity_type;
			using specific_handle = const_typed_entity_handle<E>;

			using flavour_id_type = typed_entity_flavour_id<E>;
			using flavour_type = entity_flavour<E>;

			const auto entity_type_label = format_field_name(get_type_name<E>());

			const auto total_entities = provider.template num_entities_of_type<E>();
			const auto total_flavours = provider.template num_flavours_of_type<E>();

			if (provider.skip_empty_nodes() && total_entities == 0) {
				return;
			}

			const auto node = scoped_tree_node_ex(entity_type_label);

			ImGui::NextColumn();

			if (fae_in.show_filter_buttons) {
				const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
				const auto this_type_id = entity_type_id::of<E>();

				const auto id = scoped_id(this_type_id.get_index());

				if (ImGui::Button("Ex")) {
					filter.close_type_id = this_type_id;
				}

				if (total_types > 1) {
					ImGui::SameLine();

					if (ImGui::Button("On")) {
						filter.only_type_id = this_type_id;
					}
				}

				ImGui::SameLine();
			}

			text_disabled(typesafe_sprintf("%x Flavours, %x Entities", total_flavours, total_entities));
			ImGui::NextColumn();

			if (node) {
				if (total_flavours > 1) {
					/* Perform unified flavours view, with unified entities of all flavours */

					const auto unified_flavours_node = scoped_tree_node_ex(typesafe_sprintf("%x Flavours (unified)", total_flavours));

					next_column_text();

					if (unified_flavours_node) {
						const auto& all_flavour_ids = provider.template get_all_flavour_ids<E>();

						{ 
							const auto first_flavour_id = *all_flavour_ids.begin();
							const auto& first_flavour = cosm.template get_flavour<E>(first_flavour_id);

							do_edit_flavours(first_flavour, all_flavour_ids);
						}

						if (total_entities > 0) {
							const auto unified_entities_node = scoped_tree_node_ex(typesafe_sprintf("%x Entities of %x Flavours (unified)", total_entities, all_flavour_ids.size()));

							next_column_text();

							if (unified_entities_node) {
								thread_local std::vector<entity_id_base> all_having_flavours;	
								all_having_flavours.clear();

								/*
									This could be done by just iterating over the pool and gathering all ids.
									This however will be better solution if we filter per flavours.
								*/

								for (const auto id : all_flavour_ids) {
									concatenate(
										all_having_flavours,
										provider.template get_entities_by_flavour_id<E>(id)
									);
								}

								const auto first_handle = specific_handle(cosm, *all_having_flavours.begin());
								do_edit_entities(first_handle, all_having_flavours);
							}
						}
					}
				}

				provider.template for_each_flavour<E>(
					[&](const flavour_id_type flavour_id, const flavour_type& flavour) {
						const auto flavour_label = flavour.template get<invariants::name>().name;

						const auto all_having_flavour = provider.template get_entities_by_flavour_id<E>(flavour_id.raw);

						const auto node_label = typesafe_sprintf("%x###%x", flavour_label, flavour_id.raw);
						const auto flavour_node = scoped_tree_node_ex(node_label);

						const auto num_entities_label = typesafe_sprintf("%x Entities", all_having_flavour.size());

						ImGui::NextColumn();

						if (fae_in.show_filter_buttons) {
							const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
							const auto this_type_id = entity_type_id::of<E>().get_index();

							const auto ex_label = typesafe_sprintf("Ex##%x.%x", this_type_id, flavour_id.raw);
							const auto on_label = typesafe_sprintf("On##%x.%x", this_type_id, flavour_id.raw);

							if (ImGui::Button(ex_label.c_str())) {
								filter.close_flavour_id = flavour_id;
							}

							ImGui::SameLine();

							if (!(total_flavours == 1 && total_types == 1)) {
								if (ImGui::Button(on_label.c_str())) {
									filter.only_flavour_id = flavour_id;
								}

								ImGui::SameLine();
							}
						}

						text_disabled(num_entities_label);

						ImGui::NextColumn();

						if (flavour_node) {
							ImGui::Separator();

							do_edit_flavours(flavour, flavour_id.raw);

							{
								const auto unified_entities_node = scoped_tree_node_ex(num_entities_label + " (unified)");

								next_column_text();

								if (unified_entities_node) {
									if (all_having_flavour.size() > 0) {
										const auto first_handle = specific_handle(cosm, *all_having_flavour.begin());
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
										prop_in.state.hovered_guid = guid; 
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
			}
		}	
	);

	return filter;
}
