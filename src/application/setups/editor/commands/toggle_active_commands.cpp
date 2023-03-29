#include "application/setups/editor/commands/toggle_active_commands.h"
#include "application/setups/editor/editor_setup.hpp"

void toggle_nodes_active_command::push_entry(const editor_node_id id) {
	entries.push_back({ id, false });
}

bool toggle_nodes_active_command::empty() const {
	return entries.size() == 0;
}

void toggle_nodes_active_command::redo(const editor_command_input in) {
	if (update_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& e : entries) {
		in.setup.on_node(
			e.id,
			[&](auto& typed_node, const auto& node_id) {
				(void)node_id;

				e.was_active = typed_node.active;
				typed_node.active = next_value;

				if (update_inspector) {
					in.setup.inspect_add_quiet(e.id);
				}
			}
		);
	}

	if (update_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

void toggle_nodes_active_command::undo(const editor_command_input in) {
	if (update_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& e : entries) {
		in.setup.on_node(
			e.id,
			[&](auto& typed_node, const auto& node_id) {
				(void)node_id;

				typed_node.active = e.was_active;

				if (update_inspector) {
					in.setup.inspect_add_quiet(e.id);
				}
			}
		);
	}

	if (update_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}



void toggle_layers_active_command::push_entry(const editor_layer_id id) {
	entries.push_back({ id, false });
}

bool toggle_layers_active_command::empty() const {
	return entries.size() == 0;
}

void toggle_layers_active_command::redo(const editor_command_input in) {
	if (update_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& e : entries) {
		if (auto layer = in.setup.find_layer(e.id)) {
			e.was_active = layer->editable.active;
			layer->editable.active = next_value;

			if (update_inspector) {
				in.setup.inspect_add_quiet(e.id);
			}
		}
	}

	if (update_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

void toggle_layers_active_command::undo(const editor_command_input in) {
	if (update_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& e : entries) {
		if (auto layer = in.setup.find_layer(e.id)) {
			layer->editable.active = e.was_active;

			if (update_inspector) {
				in.setup.inspect_add_quiet(e.id);
			}
		}
	}

	if (update_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}
