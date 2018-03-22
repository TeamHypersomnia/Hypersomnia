#pragma once
#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/property_editor_gui.h"

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

template <class T>
void edit_component(
	property_editor_gui& state,
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

	general_edit_properties(
		state, 
		component,
		post_new_change,
		rewrite_last_change
	);
}

template <class E>
void edit_entity(
	property_editor_gui& state,
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
				edit_component(state, component, get_index(component), handle.get_name(), command, in);
			}
		}
   	);
}
