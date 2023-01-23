#pragma once
#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/commands/edit_layer_command.h"
#include "application/setups/editor/commands/editor_command_meta.h"

void edit_layer_command::push_entry(const editor_layer_id id) {
	entries.push_back({ id, {}, {} });
}

void edit_layer_command::undo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		if (auto layer = in.setup.find_layer(entry.layer_id)) {
			layer->editable = entry.before;

			if (do_inspector) {
				in.setup.inspect_add_quiet(entry.layer_id);
			}
		}
	}

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

void edit_layer_command::redo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		if (auto layer = in.setup.find_layer(entry.layer_id)) {
			entry.before = layer->editable;
			layer->editable = entry.after;

			if (do_inspector) {
				in.setup.inspect_add_quiet(entry.layer_id);
			}
		}
	}

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}
