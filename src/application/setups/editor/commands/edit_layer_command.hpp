#pragma once
#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/commands/edit_layer_command.h"
#include "application/setups/editor/commands/editor_command_meta.h"

void edit_layer_command::undo(editor_command_input in) {
	if (auto layer = in.setup.find_layer(layer_id)) {
		after = layer->editable;
		layer->editable = before;

		in.setup.inspect_only(layer_id);
	}
}

void edit_layer_command::redo(editor_command_input in) {
	if (auto layer = in.setup.find_layer(layer_id)) {
		before = layer->editable;
		layer->editable = after;

		in.setup.inspect_only(layer_id);
	}
}
