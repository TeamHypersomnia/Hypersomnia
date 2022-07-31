#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/project/editor_layers.h"

struct toggle_nodes_visibility_command {
	editor_command_meta meta;

private:
	struct entry {
		editor_node_id id;
		bool was_visible = false;
	};

	std::vector<entry> entries;
public:

	bool next_value = false;
	bool update_inspector = true;

	std::string built_description;

	void push_entry(editor_node_id);

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}

	bool empty() const;
	auto size() const {
		return entries.size();
	}
};

struct toggle_layers_visibility_command {
	editor_command_meta meta;

private:
	struct entry {
		editor_layer_id id;
		bool was_visible = false;
	};

	std::vector<entry> entries;
public:

	bool next_value = false;
	bool update_inspector = true;

	std::string built_description;

	void push_entry(editor_layer_id);

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}

	bool empty() const;
	auto size() const {
		return entries.size();
	}
};
