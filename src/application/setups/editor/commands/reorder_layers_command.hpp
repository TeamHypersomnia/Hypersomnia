#pragma once
#include "application/setups/editor/commands/reorder_layers_command.h"
#include "augs/templates/reversion_wrapper.h"

void reorder_layers_command::undo(editor_command_input in) {
	in.setup.project.layers.order = original_order;
}

void reorder_layers_command::redo(editor_command_input in) {
	auto& layers = in.setup.project.layers.order;
	original_order = layers;

	std::vector<editor_layer_id> original_move_order;

	auto should_move = [&](const auto candidate_layer_id) {
		if (found_in(layers_to_move, candidate_layer_id)) {
			original_move_order.push_back(candidate_layer_id);
			return true;
		}

		return false;
	};

	erase_if(layers, should_move);

	const auto begin_index = std::clamp(target_index, std::size_t(0), layers.size());
	layers.insert(layers.begin() + begin_index, original_move_order.begin(), original_move_order.end());
}
