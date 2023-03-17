#pragma once
#include "application/setups/editor/gui/editor_inspector_gui.h"
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/commands/create_node_variant.h"
#include "application/setups/editor/commands/delete_nodes_command.h"

struct unpack_prefab_command {
	editor_command_meta meta;

	std::vector<create_node_variant> create_cmds;
	delete_nodes_command del_prefab_cmd;
	editor_typed_node_id<editor_prefab_node> prefab_id;

	std::string built_description;

	const auto& describe() const {
		return built_description;
	}

	void undo(editor_command_input in);
	void redo(editor_command_input in);
};

