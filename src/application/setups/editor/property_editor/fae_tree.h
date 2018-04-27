#pragma once
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_enum_radio.h"

#include "game/organization/for_each_entity_type.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/flavour_properties_editor.h"
#include "application/setups/editor/property_editor/entity_properties_editor.h"

#include "application/setups/editor/detail/checkbox_selection.h"
#include "application/setups/editor/property_editor/fae_tree_structs.h"

/*
	"fae tree" is a shorthand for "flavours and entities tree".
*/

template <class E>
void do_edit_entities_gui(
	const fae_property_editor_input in,
	const const_typed_entity_handle<E>& entity,
	std::vector<typed_entity_id<E>>&& ids
) {
	change_entity_property_command cmd;
	cmd.type_id.set<E>();
	cmd.set_affected_entities(std::move(ids));

	edit_entity(in, entity, cmd);

	ImGui::Separator();
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

	ImGui::Separator();
}

template <class F>
auto fae_tree(
	const fae_tree_input fae_in,
	F&& flavours_and_entities_provider
) {
	using namespace augs::imgui;

	const auto cpe_in = fae_in.cpe_in;
	const auto prop_in = cpe_in.prop_in;

	fae_tree_filter filter;

	auto& provider = flavours_and_entities_provider;

	std::size_t total_types = 0;

	for_each_entity_type([&](auto e){
		using E = decltype(e);

		if (provider.template num_entities_of_type<E>() > 0) {
			++total_types;
		}
	});

	auto& cosm = cpe_in.command_in.get_cosmos();
	auto& state = fae_in.state;

	const auto mode = state.view_mode;

	{
		auto child = scoped_child("fae-view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

		ImGui::Columns(2);
		ImGui::Separator();

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

				if (provider.skip_nodes_with_no_entities() && total_entities == 0) {
					return;
				}

				auto& selected_flavours = state.selected_flavours.get_for<E>();
				auto& selected_entities = state.selected_entities.get_for<E>();

				auto disabled = ::maybe_disabled_cols(prop_in.settings, total_flavours == 0);

				if (mode == fae_view_type::FLAVOURS) {
					do_select_all_checkbox(
						prop_in.settings,
						selected_flavours,
						[&provider](auto callback) {
							provider.template for_each_flavour<E>(
								[&callback](const flavour_id_type flavour_id, const flavour_type& flavour) {
									callback(flavour_id);
								}
							);
						},
						entity_type_label
					);
				}
				else if (mode == fae_view_type::ENTITIES) {
					do_select_all_checkbox(
						prop_in.settings,
						selected_entities,
						[&provider](auto callback) {
							provider.template for_each_flavour<E>(
								[&provider, callback](const flavour_id_type flavour_id, const flavour_type& flavour) {
									decltype(auto) all_having_flavour = provider.get_entities_by_flavour_id(flavour_id);

									for (const auto& e_id : all_having_flavour) {
										callback(e_id);
									}
								}
							);
						},
						entity_type_label
					);
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

				if (mode == fae_view_type::FLAVOURS) {
					text_disabled(typesafe_sprintf("%x Flavours", total_flavours));
				}
				else if (mode == fae_view_type::ENTITIES) {
					text_disabled(typesafe_sprintf("%x Entities", total_entities));
				}

				ImGui::NextColumn();

				if (node) {
					provider.template for_each_flavour<E>(
						[&](const flavour_id_type flavour_id, const flavour_type& flavour) {
							const auto flavour_label = flavour.template get<invariants::name>().name;

							decltype(auto) all_having_flavour = provider.get_entities_by_flavour_id(flavour_id);

							const auto this_type_id = entity_type_id::of<E>();
							const auto node_label = typesafe_sprintf("%x###%x", flavour_label, flavour_id.raw);
							const auto imgui_id = typesafe_sprintf("%x.%x", this_type_id.get_index(), flavour_id.raw);

							const bool is_flavour_selected = found_in(selected_flavours, flavour_id);

							auto disabled = ::maybe_disabled_cols(
								prop_in.settings, 
								(mode == fae_view_type::ENTITIES && all_having_flavour.size() == 0)
							);

							const auto flags = [&]() {
								if (mode == fae_view_type::FLAVOURS) {
									return do_selection_checkbox(
										selected_flavours,
										flavour_id,
										is_flavour_selected,
										imgui_id
									);
								}
								else {
									ensure_eq(fae_view_type::ENTITIES, mode); 

									return do_select_all_checkbox(
										prop_in.settings,
										selected_entities,
										[&all_having_flavour](auto callback) {
											for (const auto& e_id : all_having_flavour) {
												callback(e_id);
											}
										},
										imgui_id
									);
								}
							}();

							const auto flavour_node = scoped_tree_node_ex(node_label, flags);

							const auto num_entities_label = typesafe_sprintf("%x Entities", all_having_flavour.size());

							ImGui::NextColumn();

							if (fae_in.show_filter_buttons) {
								const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1));

								const auto ex_label = "Ex##" + imgui_id;
								const auto on_label = "On##" + imgui_id;

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

								if (mode == fae_view_type::FLAVOURS) {
									if (is_flavour_selected) {
										do_edit_flavours_gui(fae_in, flavour, vectorize(selected_flavours));
									}
									else {
										do_edit_flavours_gui(fae_in, flavour, { flavour_id });
									}
								}
								else if (mode == fae_view_type::ENTITIES) {
									for (const auto& e : all_having_flavour) {
										const auto typed_handle = specific_handle(cosm, e);

										const auto guid = typed_handle.get_guid();
										const auto entity_label = typesafe_sprintf("%x", guid);

										const auto is_entity_selected = found_in(selected_entities, e);

										const auto flags = do_selection_checkbox(
											selected_entities,
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
											if (is_entity_selected) {
												do_edit_entities_gui(fae_in, typed_handle, vectorize(selected_entities));
											}
											else {
												do_edit_entities_gui(fae_in, typed_handle, { e });
											}
										}
									}
								}
							}
						}
					);
				}
			}	
		);
	}

	{
		auto child = scoped_child("fae-view-switch");
		ImGui::Separator();
		enum_radio(state.view_mode, true);
	}

	return filter;
}
