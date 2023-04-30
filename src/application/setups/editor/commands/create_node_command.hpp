#pragma once
#include "application/setups/editor/commands/create_node_command.h"
#include "augs/misc/pool/pool_allocate.h"

template <class T>
inline void create_node_command<T>::undo(editor_command_input in) {
	in.setup.unregister_node_from_layer(get_node_id(), layer_id);

	{
		auto& node_pool = in.setup.project.nodes.template get_pool_for<T>();
		base::undo(node_pool);
	}

	if (create_layer.has_value()) {
		create_layer->undo(in);
	}

	if (!omit_inspector) {
		in.setup.clear_inspector();
	}
}

template <class T>
inline void create_node_command<T>::redo(editor_command_input in) {
	if (create_layer.has_value()) {
		create_layer->omit_inspector = true;
		create_layer->redo(in);

		layer_id = create_layer->get_allocated_id();
	}

	{
		created_node.unique_name = in.setup.get_free_node_name_for(created_node.unique_name);
		created_node.chronological_order = in.setup.project.nodes.next_chronological_order++;

		built_description = "Created " + created_node.unique_name;

		auto& node_pool = in.setup.project.nodes.template get_pool_for<T>();
		base::redo(node_pool, created_node);
	}

	const auto node_id = get_node_id();
	const bool registered = in.setup.register_node_in_layer(node_id, layer_id, index_in_layer);

	if (registered && !omit_inspector) {
		in.setup.inspect_only(node_id);
	}
}

template <class T>
inline editor_typed_node_id<T> create_node_command<T>::get_typed_node_id() const {
	return editor_typed_node_id<T>::from_raw(base::get_allocated_id());
}

template <class T>
inline editor_node_id create_node_command<T>::get_node_id() const {
	return get_typed_node_id().operator editor_node_id();
}
