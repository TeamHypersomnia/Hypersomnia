#pragma once
#include <cstddef>
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "application/setups/editor/project/editor_layers.h"

#include "application/setups/editor/commands/create_layer_command.h"

struct reorder_nodes_command {
	editor_command_meta meta;

	std::optional<create_layer_command> create_layer;
	editor_layer_id target_layer_id;

	std::size_t target_index = 0;
	std::vector<editor_node_id> nodes_to_move;

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}

private:
	std::unordered_map<editor_layer_id, std::vector<editor_node_id>> original_orders;
};
