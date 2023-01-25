#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "application/setups/editor/project/editor_layers.h"

#include "application/setups/editor/commands/allocating_command.h"

struct create_layer_command : allocating_command<editor_layer_id> {
	using base = allocating_command<editor_layer_id>;

	editor_layer created_layer;
	bool omit_inspector = false;
	std::size_t at_index = 0;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	auto describe() const {
		return std::string("Create ") + created_layer.unique_name;
	}

	auto get_created_id() const {
		return base::get_allocated_id();
	}
};

using editor_layer_pool_size_type = unsigned;
using layer_undo_free_input = augs::pool_undo_free_input<editor_layer_pool_size_type, editor_layer>;

struct delete_layers_command {
	struct entry {
		editor_layer_id layer_id;
		editor_layer layer_content;
		layer_undo_free_input undo_delete_input;
	};

	editor_command_meta meta;

	std::vector<entry> entries;
	std::vector<editor_layer_id> order_backup;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	void push_entry(editor_layer_id);

	std::string built_description;

	auto size() const {
		return entries.size();
	}

	bool empty() const {
		return entries.empty();
	}

	const auto& describe() const {
		return built_description;
	}
};
