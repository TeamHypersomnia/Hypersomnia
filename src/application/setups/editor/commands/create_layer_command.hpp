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

	const auto new_id = base::get_allocated_id();
	layers.order.insert(layers.order.begin(), new_id);
	in.setup.inspect_only(new_id);
}
