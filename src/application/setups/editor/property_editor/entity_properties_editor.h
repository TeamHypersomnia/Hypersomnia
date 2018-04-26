#pragma once
#include "application/setups/editor/property_editor/compare_all_fields_to.h"

#include "application/setups/editor/detail/field_address.h"
#include "application/setups/editor/property_editor/general_edit_properties.h"
#include "application/setups/editor/detail/format_struct_name.h"

template <class T>
void edit_component(
	const fae_property_editor_input in,
	const T& component,
	const unsigned component_id, 
	const std::string& entity_name,
	const change_entity_property_command& command
) {
	using command_type = std::decay_t<decltype(command)>;
	using namespace augs::imgui;

	const auto property_location = [&]() {
		const auto component_name = format_struct_name(component);
		return typesafe_sprintf(" (in %x of %x#%x)", component_name, entity_name);
	}();

	const auto cpe_in = in.cpe_in;
	const auto cmd_in = cpe_in.command_in;

	/* Linker error fix */
	auto& history = cmd_in.folder.history;

	auto post_new_change = [&](
		const auto& description,
		const auto field_id,
		const auto& new_content
	) {
		auto cmd = command;

		cmd.property_id = entity_property_id { component_id, field_id };
		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, cmd_in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<command_type>(std::addressof(last))) {
			cmd->built_description = description + property_location;
			cmd->rewrite_change(augs::to_bytes(new_content), cmd_in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	const auto& cosm = cmd_in.get_cosmos();

	general_edit_properties(
		cpe_in.prop_in, 
		component,
		post_new_change,
		rewrite_last_change,
		[&](const auto& first, const field_address field_id) {
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
	const fae_property_editor_input in,
	const const_typed_entity_handle<E> handle,
	const change_entity_property_command& command
) {
	using namespace augs::imgui;

	auto get_index = [](const auto& comp) {
		return index_in_list_v<std::decay_t<decltype(comp)>, components_of<E>>;
	};

	for_each_through_std_get(
		handle.get().components,
		[&](const auto& component) {
			using T = std::decay_t<decltype(component)>;

			const auto component_label = format_struct_name(component) + " component";
			const auto node = scoped_tree_node_ex(component_label);

			next_column_text();

			if (node) {
				edit_component(in, component, get_index(component), handle.get_name(), command);
			}
		}
   	);
}
