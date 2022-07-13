#pragma once
#include "application/setups/editor/commands/create_layer_command.h"

void create_layer_command::undo(editor_command_input in) {
	auto& layers = in.setup.project.layers;
	base::undo(layers.pool);

	erase_element(layers.order, base::get_allocated_id());
}

void create_layer_command::redo(editor_command_input in) {
	auto& layers = in.setup.project.layers;
	auto& new_layer = base::redo(layers.pool);
	new_layer.unique_name = chosen_name;

	layers.order.insert(layers.order.begin(), base::get_allocated_id());
}
