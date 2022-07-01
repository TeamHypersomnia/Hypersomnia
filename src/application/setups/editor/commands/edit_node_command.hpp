#pragma once
#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/commands/edit_node_command.h"
#include "application/setups/editor/commands/editor_command_meta.h"

template <class T>
void edit_node_command<T>::undo(editor_command_input in) {
	if (auto node = in.setup.find_node(node_id)) {
		after = node->editable;
		node->editable = before;

		in.setup.inspect(editor_node_id(node_id));
	}
}

template <class T>
void edit_node_command<T>::redo(editor_command_input in) {
	if (auto node = in.setup.find_node(node_id)) {
		before = node->editable;
		node->editable = after;

		in.setup.inspect(editor_node_id(node_id));
	}
}
