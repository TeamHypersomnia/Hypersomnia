#pragma once
#include "application/setups/editor/commands/create_layer_command.h"

void create_layer_command::undo(editor_command_input in) {
	auto& layers = in.setup.project.layers;
	base::undo(layers.pool);

	erase_element(layers.order, base::get_allocated_id());

	if (!omit_inspector) {
		in.setup.clear_inspector();
	}
}

void create_layer_command::redo(editor_command_input in) {
	auto& layers = in.setup.project.layers;
	auto& new_layer = base::redo(layers.pool);
	new_layer = created_layer;

	const auto new_id = base::get_allocated_id();
	layers.order.insert(layers.order.begin() + at_index, new_id);

	if (!omit_inspector) {
		in.setup.inspect_only(new_id);
		in.setup.scroll_once_to(new_id);
	}
}

void delete_layers_command::push_entry(editor_layer_id layer_id) {
	entries.push_back({ layer_id, {}, {} });
}

void delete_layers_command::redo(editor_command_input in) {
	in.setup.clear_inspector();

	auto& project_layers = in.setup.project.layers;
	auto& pool = project_layers.pool;

	order_backup = project_layers.order;

	for (auto& entry : entries) {
		const auto found_layer = pool.find(entry.layer_id);
		ensure(found_layer != nullptr);
		entry.layer_content = *found_layer;

		const auto delete_input = pool.free(entry.layer_id);
		ensure(delete_input.has_value());
		entry.undo_delete_input = *delete_input;

		erase_element(project_layers.order, entry.layer_id);
	}
}

void delete_layers_command::undo(editor_command_input in) {
	in.setup.clear_inspector();

	auto& project_layers = in.setup.project.layers;
	auto& pool = project_layers.pool;

	for (auto& entry : reverse(entries)) {
		const auto [undone_id, object] = pool.undo_free(entry.undo_delete_input, std::move(entry.layer_content));
		ensure(undone_id == entry.layer_id);

		in.setup.inspect_add_quiet(entry.layer_id);
	}

	project_layers.order = order_backup;
	in.setup.after_quietly_adding_inspected();
}
