#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/project/editor_layers.h"

#include "application/setups/editor/commands/allocating_command.h"

struct reorder_layers_command {
	editor_command_meta meta;

	std::size_t target_index = 0;
	std::vector<editor_layer_id> layers_to_move;

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}

private:
	std::vector<editor_layer_id> original_order;
};
