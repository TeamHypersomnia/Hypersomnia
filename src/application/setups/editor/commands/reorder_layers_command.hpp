#pragma once
#include <cstddef>
#include "application/setups/editor/commands/reorder_layers_command.h"
#include "augs/templates/reversion_wrapper.h"

void reorder_layers_command::undo(editor_command_input in) {
	auto& layers = in.setup.project.layers;
	layers.order = original_order;
	layers.scratchpad = original_scratchpad;
}

void reorder_layers_command::redo(editor_command_input in) {
	auto& layers = in.setup.project.layers;
	original_order = layers.order;
	original_scratchpad = layers.scratchpad;

	/*
		Keep the moved layers in the order they had, the scratchpad's after the others'.
	*/

	std::vector<editor_layer_id> original_move_order;

	auto should_move = [&](const auto candidate_layer_id) {
		if (found_in(layers_to_move, candidate_layer_id)) {
			original_move_order.push_back(candidate_layer_id);
			return true;
		}

		return false;
	};

	erase_if(layers.order, should_move);
	erase_if(layers.scratchpad, should_move);

	auto& target = to_scratchpad ? layers.scratchpad : layers.order;

	const auto begin_index = std::clamp(target_index, std::size_t(0), target.size());
	target.insert(target.begin() + begin_index, original_move_order.begin(), original_move_order.end());
}
