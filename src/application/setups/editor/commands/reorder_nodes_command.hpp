#pragma once
#include "application/setups/editor/commands/reorder_nodes_command.h"
#include "augs/templates/reversion_wrapper.h"
#include "application/setups/editor/commands/create_layer_command.hpp"

inline void reorder_nodes_command::undo(editor_command_input in) {
	if (nodes_to_move.empty()) {
		return;
	}

	if (create_layer.has_value()) {
		create_layer->undo(in);
	}

	for (const auto& original : original_orders) {
		auto layer = in.setup.find_layer(original.first);

		ensure(layer != nullptr);

		layer->hierarchy.nodes = original.second;
	}

	original_orders.clear();

	in.setup.inspect_only(nodes_to_move);

	if (!in.skip_inspector) {
		in.setup.scroll_once_to(nodes_to_move[0]);
	}
}

inline void reorder_nodes_command::redo(editor_command_input in) {
	if (nodes_to_move.empty()) {
		return;
	}

	std::vector<editor_node_id> original_move_order;

	auto should_move = [&](const auto candidate_node_id) {
		if (found_in(nodes_to_move, candidate_node_id)) {
			original_move_order.push_back(candidate_node_id);
			return true;
		}

		return false;
	};

	for (const auto& id : in.setup.get_layers()) {
		auto layer = in.setup.find_layer(id);
		ensure(layer != nullptr);

		auto& nodes = layer->hierarchy.nodes;
		original_orders[id] = nodes;
		erase_if(nodes, should_move);
	}

	if (create_layer.has_value()) {
		create_layer->omit_inspector = true;
		create_layer->redo(in);

		target_layer_id = create_layer->get_allocated_id();

		if (!in.skip_inspector) {
			in.setup.scroll_once_to(target_layer_id);
		}
	}
	else {
		if (!in.skip_inspector) {
			in.setup.scroll_once_to(nodes_to_move[0]);
		}
	}

	auto target_layer = in.setup.find_layer(target_layer_id);
	ensure(target_layer != nullptr);

	auto& nodes = target_layer->hierarchy.nodes;
	const auto begin_index = std::clamp(target_index, std::size_t(0), nodes.size());
	nodes.insert(nodes.begin() + begin_index, original_move_order.begin(), original_move_order.end());

	in.setup.inspect_only(nodes_to_move);
}
