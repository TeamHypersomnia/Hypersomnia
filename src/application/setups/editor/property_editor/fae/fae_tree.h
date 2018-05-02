#pragma once
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "game/organization/for_each_entity_type.h"

#include "application/setups/editor/commands/flavour_commands.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/fae/flavour_properties_editor.h"
#include "application/setups/editor/property_editor/fae/entity_properties_editor.h"

#include "application/setups/editor/detail/other_styles.h"
#include "application/setups/editor/detail/checkbox_selection.h"
#include "application/setups/editor/property_editor/fae/fae_tree_structs.h"

#include "application/setups/editor/detail/find_locations_that_use.h"
#include "application/setups/editor/property_editor/fae/ex_on_buttons.h"

template <class E>
void do_edit_entities_gui(
	const fae_property_editor_input in,
	const cref_typed_entity_handle<E>& entity,
	std::vector<typed_entity_id<E>>&& ids
) {
	change_entity_property_command cmd;
	cmd.type_id.set<E>();
	cmd.set_affected_entities(std::move(ids));

	edit_entity(in, entity, cmd);
}

template <class E>
void do_edit_flavours_gui(
	const fae_property_editor_input in,
	const entity_flavour<E>& flavour,
	std::vector<typed_entity_flavour_id<E>>&& ids
) {
	change_flavour_property_command cmd;
	cmd.type_id.set<E>();
	cmd.set_affected_flavours(std::move(ids));

	edit_flavour(in, flavour, cmd);
}

template <class F>
auto detail_get_total_types(F& provider) {
	std::size_t result = 0;

	for_each_entity_type([&](auto e){
		using E = decltype(e);

		if (provider.template num_entities_of_type<E>() > 0) {
			++result;
		}
	});

	return result;
}

template <class F, class T>
auto tree_of_flavours(
	const fae_tree_input fae_in,
	F provider,
	T& all_ticked_flavours
) {
	using namespace augs::imgui;

	const auto cpe_in = fae_in.cpe_in;
	const auto prop_in = cpe_in.prop_in;
	const auto settings = prop_in.settings;

	fae_tree_output output;
	auto& filter = output.filter;

	const auto total_types = detail_get_total_types(provider);

	{
		auto child = scoped_child("fae-view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

		ImGui::Columns(2);
		ImGui::Separator();

		for_each_entity_type(
			[&](auto e) {
				using E = decltype(e);

				using flavour_id_type = typed_entity_flavour_id<E>;
				using flavour_type = entity_flavour<E>;

				if (provider.template should_skip_type<E>()) {
					return;
				}

				auto& ticked_flavours = all_ticked_flavours.template get_for<E>();

				static const auto entity_type_label = format_field_name(get_type_name<E>());
				const auto this_type_id = entity_type_id::of<E>();

				const auto total_flavours = provider.template num_flavours_of_type<E>();

				const auto flags = [&]() {
					auto disabled = ::maybe_disabled_cols(settings, total_flavours == 0);

					return do_tick_all_checkbox(
						settings,
						ticked_flavours,
						[&provider](auto callback) {
							provider.template for_each_flavour<E>(
								[&callback](const flavour_id_type flavour_id, const flavour_type&) {
									callback(flavour_id);
								}
							);
						},
						entity_type_label
					);
				}();

				auto node = scoped_tree_node_ex(entity_type_label, flags);

				ImGui::NextColumn();

				ex_on_buttons<E>(
					fae_in,
					total_types,
					filter
				);

				text_disabled(typesafe_sprintf("%x Flavours", total_flavours));
				ImGui::NextColumn();
				
				if (node) {
					provider.template for_each_flavour<E>(
						[&](const flavour_id_type flavour_id, const flavour_type& flavour) {
							const auto flavour_label = flavour.template get<invariants::name>().name;

							const auto node_label = typesafe_sprintf("%x###%x", flavour_label, flavour_id.raw);
							const auto imgui_id = typesafe_sprintf("%x.%x", this_type_id.get_index(), flavour_id.raw);

							const bool is_flavour_selected = found_in(ticked_flavours, flavour_id);

							const auto flags = do_selection_checkbox(
								ticked_flavours,
								flavour_id,
								is_flavour_selected,
								imgui_id
							);

							thread_local std::vector<std::string> locations;
							locations.clear();

							find_locations_that_use(
								flavour_id,
								*cpe_in.command_in.folder.work,
								[](const std::string& location) {
									locations.push_back(location);
								}
							);

							if (fae_in.show_flavour_control_buttons) {
								const auto scoped_style = in_line_button_style();

								{
									const auto button_label = "D##" + imgui_id;

									if (ImGui::Button(button_label.c_str())) {
										duplicate_flavour_command cmd;
										cmd.set_duplicated_id(flavour_id);
										post_editor_command(cpe_in.command_in, std::move(cmd));
									}
								}

								ImGui::SameLine();

								{
									auto disabled_scope = ::maybe_disabled_cols(settings, locations.size() > 0);

									const auto button_label = "-##" + imgui_id;

									if (ImGui::Button(button_label.c_str())) {
										delete_flavour_command cmd;
										cmd.set_deleted_id(flavour_id);
										post_editor_command(cpe_in.command_in, std::move(cmd));
									}
								}

								ImGui::SameLine();
							}

							const auto flavour_node = scoped_tree_node_ex(node_label, flags);

							ImGui::NextColumn();

							if (fae_in.show_locations_using_flavour) {
								if (locations.empty()) {
									text_disabled("(Used nowhere)");
								}
								else if (const auto locations_node = scoped_tree_node_ex(typesafe_sprintf("Locations that use##%x", imgui_id), flags)) {
									for (const auto& l : locations) {
										text(l);
									}
								}
							}
							else {
								ex_on_buttons(
									fae_in,
									flavour_id,
									total_types,
									total_flavours,
									imgui_id,
									filter
								);
							}

							ImGui::NextColumn();

							if (flavour_node) {
								ImGui::Separator();

								if (is_flavour_selected) {
									do_edit_flavours_gui(fae_in, flavour, { ticked_flavours.begin(), ticked_flavours.end() });
								}
								else {
									do_edit_flavours_gui(fae_in, flavour, { flavour_id });
								}

								ImGui::Separator();
							}
						}
					);

					if (fae_in.show_flavour_control_buttons) {
						const auto scoped_style = in_line_button_style();
						const auto button_label = "+##" + entity_type_label;

						if (ImGui::Button(button_label.c_str())) {
							create_flavour_command cmd;
							cmd.type_id = this_type_id;
							post_editor_command(cpe_in.command_in, std::move(cmd));
						}
					}
				}
			}	
		);
	}

	return output;
}

template <class F, class T>
auto tree_of_entities(
	const fae_tree_input fae_in,
	entities_tree_state& state,
	F provider,
	T& all_ticked_entities
) {
	using namespace augs::imgui;

	const auto cpe_in = fae_in.cpe_in;
	const auto prop_in = cpe_in.prop_in;
	const auto settings = prop_in.settings;

	fae_tree_output output;
	auto& filter = output.filter;

	const auto total_types = detail_get_total_types(provider);

	{
		auto child = scoped_child("fae-view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

		ImGui::Columns(2);
		ImGui::Separator();

		const auto& cosm = cpe_in.command_in.get_cosmos();

		for_each_entity_type(
			[&](auto e) {
				using E = decltype(e);

				using flavour_id_type = typed_entity_flavour_id<E>;
				using flavour_type = entity_flavour<E>;

				if (provider.template should_skip_type<E>()) {
					return;
				}

				auto& ticked_entities = all_ticked_entities.template get_for<E>();

				static const auto entity_type_label = format_field_name(get_type_name<E>());

				const auto total_entities = provider.template num_entities_of_type<E>();

				const auto flags = [&]() {
					auto disabled = ::maybe_disabled_cols(settings, total_entities == 0);

					return do_tick_all_checkbox(
						settings,
						ticked_entities,
						[&provider](auto callback) {
							provider.template for_each_flavour<E>(
								[&provider, callback](const flavour_id_type flavour_id, const flavour_type&) {
									decltype(auto) all_having_flavour = provider.get_entities_by_flavour_id(flavour_id);

									for (const auto& e_id : all_having_flavour) {
										callback(e_id);
									}
								}
							);
						},
						entity_type_label
					);
				}();

				const auto total_flavours = provider.template num_flavours_of_type<E>();

				auto disabled = ::maybe_disabled_cols(settings, !total_entities && !total_flavours);

				auto node = scoped_tree_node_ex(entity_type_label, flags);

				ImGui::NextColumn();

				ex_on_buttons<E>(
					fae_in,
					total_types,
					filter
				);

				text_disabled(typesafe_sprintf("%x Entities", total_entities));
				ImGui::NextColumn();

				if (node) {
					provider.template for_each_flavour<E>(
						[&](const flavour_id_type flavour_id, const flavour_type& flavour) {
							const auto flavour_label = flavour.template get<invariants::name>().name;

							decltype(auto) all_having_flavour = provider.get_entities_by_flavour_id(flavour_id);

							const auto this_type_id = entity_type_id::of<E>();
							const auto node_label = typesafe_sprintf("%x###%x", flavour_label, flavour_id.raw);
							const auto imgui_id = typesafe_sprintf("%x.%x", this_type_id.get_index(), flavour_id.raw);

							const bool no_entities_here = all_having_flavour.empty();

							const auto flags = [&]() {
								auto disabled = ::maybe_disabled_cols(settings, no_entities_here);

								return do_tick_all_checkbox(
									settings,
									ticked_entities,
									[&all_having_flavour](auto callback) {
										for (const auto& e_id : all_having_flavour) {
											callback(e_id);
										}
									},
									imgui_id
								);
							}();

							{
								const auto scoped_style = in_line_button_style();

								{
									const auto button_label = "I##" + imgui_id;

									if (ImGui::Button(button_label.c_str())) {
										output.instantiate_id = flavour_id;
									}
								}

								ImGui::SameLine();
							}

							auto disabled = ::maybe_disabled_cols(settings, no_entities_here);

							const auto flavour_node = scoped_tree_node_ex(node_label, flags);

							const auto num_entities_label = typesafe_sprintf("%x Entities", all_having_flavour.size());

							ImGui::NextColumn();

							ex_on_buttons(
								fae_in,
								flavour_id,
								total_types,
								provider.template num_flavours_of_type<E>(),
								imgui_id,
								filter
							);

							text_disabled(num_entities_label);
							ImGui::NextColumn();

							if (flavour_node) {
								for (const auto& e : all_having_flavour) {
									const auto typed_handle = *cosm[e];

									const auto guid = typed_handle.get_guid();
									const auto entity_label = typesafe_sprintf("%x", guid);

									const auto is_entity_selected = found_in(ticked_entities, e);

									const auto flags = do_selection_checkbox(
										ticked_entities,
										e,
										is_entity_selected,
										guid
									);

									const auto entity_node = scoped_tree_node_ex(entity_label, flags);

									if (ImGui::IsItemHovered()) {
										state.hovered_guid = guid; 
									}

									next_column_text();

									if (entity_node) {
										ImGui::Separator();

										if (is_entity_selected) {
											do_edit_entities_gui(fae_in, typed_handle, { ticked_entities.begin(), ticked_entities.end() });
										}
										else {
											do_edit_entities_gui(fae_in, typed_handle, { e });
										}

										ImGui::Separator();
									}
								}
							}
						}
					);
				}
			}	
		);
	}

	return output;
}
