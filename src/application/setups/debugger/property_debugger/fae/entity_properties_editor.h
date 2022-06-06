#pragma once
#include "application/setups/debugger/property_debugger/compare_all_fields_to.h"

#include "application/setups/debugger/detail/field_address.h"
#include "application/setups/debugger/property_debugger/general_edit_properties.h"
#include "application/setups/debugger/detail/format_struct_name.h"
#include "application/setups/debugger/detail/rewrite_last_change.h"

template <class T>
void edit_component(
	const fae_property_debugger_input in,
	const T& component,
	const unsigned component_id, 
	const std::string& entity_name,
	const change_entity_property_command& command
) {
	using cmd_type = remove_cref<decltype(command)>;
	using field_type_id = property_field_type_id_t<cmd_type>;

	using namespace augs::imgui;

	const auto property_location = [&]() {
		const auto component_name = format_struct_name(component);
		return typesafe_sprintf(" (in %x of %x#%x)", component_name, entity_name);
	}();

	const auto cpe_in = in.cpe_in;
	const auto cmd_in = cpe_in.command_in;

	/* Linker error fix */
	auto& history = cmd_in.get_history();

	auto post_new_change = [&](
		const auto& description,
		const auto& field_id,
		const auto& new_content
	) {
		auto cmd = command;

		cmd.property_id = entity_property_id { component_id, field_id };
		augs::assign_bytes(cmd.value_after_change, new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, cmd_in);
	};

	auto rewrite_last_change = make_rewrite_last_change<cmd_type>(
		property_location,
		cmd_in
	);

	const auto& cosm = cmd_in.get_cosmos();

	general_edit_properties<field_type_id>(
		cpe_in.prop_in, 
		component,
		post_new_change,
		rewrite_last_change,
		[&](const auto& first, const auto& field_id) {
			return compare_all_fields_to(
				first,
				entity_property_id { component_id, field_id }, 
				cosm, 
				command.type_id, 
				command.affected_entities
			);
		}
	);
}

template <class E>
void edit_entity(
	const fae_property_debugger_input in,
	const cref_typed_entity_handle<E> handle,
	const change_entity_property_command& command
) {
	using namespace augs::imgui;

	auto get_index = [](const auto& comp) {
		return index_in_list_v<remove_cref<decltype(comp)>, components_of<E>>;
	};

	text_disabled(typesafe_sprintf("(%x)", handle.get_id()));

	for_each_through_std_get(
		handle.get().component_state,
		[&](const auto& component) {
			const auto component_label = format_struct_name(component) + " component";
			const auto node = scoped_tree_node_ex(component_label);

			next_column_text();

			if (node) {
				edit_component(in, component, get_index(component), handle.get_name(), command);
			}
		}
   	);
}
