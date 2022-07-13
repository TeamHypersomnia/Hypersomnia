#pragma once
#include "application/setups/editor/commands/reorder_nodes_command.h"
#include "augs/templates/reversion_wrapper.h"

void reorder_nodes_command::undo(editor_command_input in) {
	auto source_layer = in.setup.find_layer(source_layer_id);
	auto target_layer = in.setup.find_layer(target_layer_id);

	ensure(source_layer != nullptr);
	ensure(target_layer != nullptr);

	source_layer->hierarchy.nodes = original_order_in_source;
	target_layer->hierarchy.nodes = original_order_in_target;
}

void reorder_nodes_command::redo(editor_command_input in) {
	auto source_layer = in.setup.find_layer(source_layer_id);
	auto target_layer = in.setup.find_layer(target_layer_id);

	ensure(source_layer != nullptr);
	ensure(target_layer != nullptr);

	original_order_in_source = source_layer->hierarchy.nodes;
	original_order_in_target = target_layer->hierarchy.nodes;

	std::vector<editor_node_id> original_move_order;

	auto should_move = [&](const auto candidate_node_id) {
		if (found_in(nodes_to_move, candidate_node_id)) {
			original_move_order.push_back(candidate_node_id);
			return true;
		}

		return false;
	};

	erase_if(source_layer->hierarchy.nodes, should_move);

	for (const auto node_id : reverse(original_move_order)) {
		auto& nodes = target_layer->hierarchy.nodes;

		nodes.insert(nodes.begin() + target_index, node_id);
	}
}
