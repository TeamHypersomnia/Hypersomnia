#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/commands/editor_command_meta.h"

struct edit_layer_command {
	using editable_type = editor_layer_editable;

	editor_command_meta meta;

	editor_layer_id layer_id;

	editable_type before;
	editable_type after;

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}
};
