#pragma once
#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/commands/edit_node_command.h"
#include "application/setups/editor/commands/editor_command_meta.h"

template <class T>
void edit_node_command<T>::push_entry(const editor_typed_node_id<T> id) {
	entries.push_back({ id, {}, {} });
}

template <class T>
void edit_node_command<T>::undo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		if (auto node = in.setup.find_node(entry.node_id)) {
			node->editable = entry.before;

			if (do_inspector) {
				in.setup.inspect_add_quiet(editor_node_id(entry.node_id));
			}
		}
	}

	if (do_inspector) {
		in.setup.set_inspector_tab(inspected_node_tab_type::NODE);
		in.setup.after_quietly_adding_inspected();
	}
}

template <class T>
void edit_node_command<T>::redo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		if (auto node = in.setup.find_node(entry.node_id)) {
			entry.before = node->editable;
			node->editable = entry.after;

			if (do_inspector) {
				in.setup.inspect_only(editor_node_id(entry.node_id));
			}
		}
	}

	if (do_inspector) {
		in.setup.set_inspector_tab(inspected_node_tab_type::NODE);
		in.setup.after_quietly_adding_inspected();
	}
}

void rename_node_command::push_entry(const editor_node_id id) {
	entries.push_back({ id, {} });
}

void rename_node_command::undo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		in.setup.on_node(
			entry.node_id,
			[&](auto& node, const auto id) {
				(void)id;

				node.unique_name = entry.before;

				if (do_inspector) {
					in.setup.inspect_add_quiet(entry.node_id);
				}
			}
		);
	}

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

void rename_node_command::redo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		in.setup.on_node(
			entry.node_id,
			[&](auto& node, const auto id) {
				(void)id;

				entry.before = node.unique_name;
				node.unique_name = in.setup.get_free_node_name_for(after);

				if (do_inspector) {
					in.setup.inspect_add_quiet(entry.node_id);
				}
			}
		);
	}

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

void rename_layer_command::push_entry(const editor_layer_id id) {
	entries.push_back({ id, {} });
}

void rename_layer_command::undo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		if (const auto layer = in.setup.find_layer(entry.layer_id)) {
			layer->unique_name = entry.before;

			if (do_inspector) {
				in.setup.inspect_add_quiet(entry.layer_id);
			}
		}
	}

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

void rename_layer_command::redo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

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
