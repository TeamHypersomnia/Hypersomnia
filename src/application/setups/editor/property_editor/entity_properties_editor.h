#pragma once
#include "application/setups/editor/property_editor/compare_all_fields_to.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/general_edit_properties.h"

template <class T>
auto get_component_stem(const T&) {
	auto result = format_field_name(get_type_name_strip_namespace<T>());
	result[0] = std::toupper(result[0]);

	/* These look ugly with automated names */

	if constexpr(std::is_same_v<T, components::transform>) {
		result = "Transform";
	}	

	return result;
}

struct component_field_eq_predicate {
	const cosmos& cosm;
	const unsigned component_id;
	const entity_type_id type_id;
	const affected_entities_type& ids;

	template <class M>
	bool compare(
		const M& first,
		const field_address field_id
	) const {
		if (ids.size() == 1) {
			return true;
		}

		entity_property_id property_id;
		property_id.component_id = component_id;
		property_id.field = field_id;

		return compare_all_fields_to(first, property_id, type_id, cosm, ids);
	}
};

template <class T>
void edit_component(
	const property_editor_input prop_in,
	const T& component,
	const unsigned component_id, 
	const std::string& entity_name,
	const change_entity_property_command& command,
   	const editor_command_input in
) {
	using command_type = std::decay_t<decltype(command)>;
	using namespace augs::imgui;

	auto make_property_id = [&](const field_address field) {
		entity_property_id result;

		result.component_id = component_id; 
		result.field = field;

		return result;
	};

	const auto property_location = [&]() {
		const auto component_name = get_component_stem(component);
		return typesafe_sprintf(" (in %x of %x#%x)", component_name, entity_name);
	}();

	/* Linker error fix */
	auto& history = in.folder.history;

	auto post_new_change = [&](
		const auto& description,
		const auto field_id,
		const auto& new_content
	) {
		auto cmd = command;

		cmd.property_id = make_property_id(field_id);
		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<command_type>(std::addressof(last))) {
			cmd->built_description = description + property_location;
			cmd->rewrite_change(augs::to_bytes(new_content), in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	const auto& cosm = in.get_cosmos();

	general_edit_properties(
		prop_in, 
		component,
		post_new_change,
		rewrite_last_change,
		component_field_eq_predicate { 
			cosm, component_id, command.type_id, command.affected_entities 
		}
	);
}

template <class E>
void edit_entity(
	const property_editor_input prop_in,
	const const_typed_entity_handle<E> handle,
	const change_entity_property_command& command,
   	const editor_command_input in
) {
	using namespace augs::imgui;

	auto get_index = [](const auto& comp) {
		return index_in_list_v<std::decay_t<decltype(comp)>, components_of<E>>;
	};

	for_each_through_std_get(
		handle.get().components,
		[&](const auto& component) {
			using T = std::decay_t<decltype(component)>;

			const auto component_label = get_component_stem(component) + " component";
			const auto node = scoped_tree_node_ex(component_label);

			next_column_text();

			if (node) {
				edit_component(prop_in, component, get_index(component), handle.get_name(), command, in);
			}
		}
   	);
}
