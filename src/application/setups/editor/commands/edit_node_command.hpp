#pragma once
#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/commands/edit_node_command.h"
#include "application/setups/editor/commands/editor_command_meta.h"

template <class T>
void edit_node_command<T>::undo(editor_command_input in) {
	if (auto node = in.setup.find_node(node_id)) {
		after = node->editable;
		node->editable = before;

		in.setup.inspect_only(editor_node_id(node_id));
	}
}

template <class T>
void edit_node_command<T>::redo(editor_command_input in) {
	if (auto node = in.setup.find_node(node_id)) {
		before = node->editable;
		node->editable = after;

		in.setup.inspect_only(editor_node_id(node_id));
	}
}

void rename_node_command::undo(editor_command_input in) {
	in.setup.on_node(
		node_id,
		[&](auto& node, const auto id) {
			(void)id;

			after = node.unique_name;
			node.unique_name = before;

			in.setup.inspect_only(node_id);
		}
	);
}

void rename_node_command::redo(editor_command_input in) {
	in.setup.on_node(
		node_id,
		[&](auto& node, const auto id) {
			(void)id;

			before = node.unique_name;
			node.unique_name = after;

			if (!in.setup.get_history().executed_new()) {
				in.setup.inspect_only(node_id);
			}
		}
	);
}

void rename_layer_command::undo(editor_command_input in) {
	if (const auto layer = in.setup.find_layer(layer_id)) {
		after = layer->unique_name;
		layer->unique_name = before;

		in.setup.inspect_only(layer_id);
	}
}

void rename_layer_command::redo(editor_command_input in) {
	if (const auto layer = in.setup.find_layer(layer_id)) {
		before = layer->unique_name;
		layer->unique_name = after;

		/* 
			If we inspect on first execution,
			pressing up arrow while renaming doesn't result in the inspector focus going up
			since the inspected element is overridden here to the renamed element again.
		*/

		if (!in.setup.get_history().executed_new()) {
			in.setup.inspect_only(layer_id);
		}
	}
}
