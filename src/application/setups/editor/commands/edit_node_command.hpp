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

void rename_layer_command::push_entry(const editor_layer_id id) {
	entries.push_back({ id, {} });
}

void rename_layer_command::undo(editor_command_input in) {
	in.setup.clear_inspector();

	for (auto& entry : entries) {
		if (const auto layer = in.setup.find_layer(entry.layer_id)) {
			layer->unique_name = entry.before;

			in.setup.inspect_add_quiet(entry.layer_id);
		}
	}

	in.setup.after_quietly_adding_inspected();
}

void rename_layer_command::redo(editor_command_input in) {
	const bool do_inspector = !in.setup.get_history().executed_new();

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		if (const auto layer = in.setup.find_layer(entry.layer_id)) {
			entry.before = layer->unique_name;
			layer->unique_name = in.setup.get_free_layer_name_for(after);

			/* 
				If we inspect on first execution,
				pressing up arrow while renaming doesn't result in the inspector focus going up
				since the inspected element is overridden here to the renamed element again.
			*/

			if (do_inspector) {
				in.setup.inspect_add_quiet(entry.layer_id);
			}
		}
	}

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}
