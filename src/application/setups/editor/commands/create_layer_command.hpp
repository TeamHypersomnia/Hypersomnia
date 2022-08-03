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
	new_layer = created_layer;

	const auto new_id = base::get_allocated_id();
	layers.order.insert(layers.order.begin() + at_index, new_id);

	if (!omit_inspector) {
		in.setup.inspect_only(new_id);
		in.setup.scroll_once_to(new_id);
	}
}
