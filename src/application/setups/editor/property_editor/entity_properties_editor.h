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

template <class T, class E>
void edit_component(
	property_editor_gui& state,
	const T& component,
	const E handle,
   	const editor_command_input in
) {
	using namespace augs::imgui;

	auto make_property_id = [&](const field_address field) {
		entity_property_id result;

		result.subject_id = handle.get_id();
		result.component_id = index_in_list_v<T, decltype(handle.get().components)>;
		result.field = field;

		return result;
	};

	const auto property_location = [&]() {
		const auto entity_name = handle.get_name();
		const auto component_name = get_component_stem(component);

		return typesafe_sprintf(" (in %x of %x)", component_name, entity_name);
	}();

	/* Linker error fix */
	auto& history = in.folder.history;

	auto post_new_change = [&](
		const auto& description,
		const entity_property_id property_id,
		const auto& new_content
	) {
		change_entity_property_command cmd;
		cmd.property_id = property_id;

		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<change_entity_property_command>(std::addressof(last))) {
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
		make_property_id,
		post_new_change,
		rewrite_last_change
	);
}

template <class E>
void edit_entity(
	property_editor_gui& state,
	const E handle,
   	const editor_command_input in
) {
	using namespace augs::imgui;

	for_each_through_std_get(
		handle.get().components,
		[&](const auto& component) {
			using T = std::decay_t<decltype(component)>;

			const auto component_label = get_component_stem(component) + " component";

			if (const auto node = scoped_tree_node_ex(component_label.c_str())) {
				edit_component(state, component, handle, in);
			}
	
			next_column_text();
		}
   	);
}
