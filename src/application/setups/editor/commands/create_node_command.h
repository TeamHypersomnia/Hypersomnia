#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "application/setups/editor/project/editor_layers.h"

#include "application/setups/editor/commands/allocating_command.h"
#include "application/setups/editor/commands/create_layer_command.h"

template <class T>
struct create_node_command : allocating_command<editor_node_pool_id> {
	using base = allocating_command<editor_node_pool_id>;

	std::optional<create_layer_command> create_layer;

	T created_node;
	editor_layer_id layer_id;
	std::size_t index_in_layer = 0;

	std::string built_description;
	bool omit_inspector = false;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}

	editor_node_id get_node_id() const;
	editor_typed_node_id<T> get_typed_node_id() const;
};
