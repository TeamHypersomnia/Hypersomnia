#pragma once
#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/commands/edit_resource_command.h"
#include "application/setups/editor/commands/editor_command_meta.h"

template <class T>
void edit_resource_command<T>::undo(editor_command_input in) {
	if (auto resource = in.setup.find_resource(resource_id)) {
		after = resource->editable;
		resource->editable = before;

		if (override_inspector_state != std::nullopt) {
			in.setup.inspect_only(*override_inspector_state);
		}
		else {
			in.setup.inspect_only(editor_resource_id(resource_id));
		}
	}
}

template <class T>
void edit_resource_command<T>::redo(editor_command_input in) {
	if (auto resource = in.setup.find_resource(resource_id)) {
		before = resource->editable;
		resource->editable = after;

		if (override_inspector_state != std::nullopt) {
			in.setup.inspect_only(*override_inspector_state);
		}
		else {
			in.setup.inspect_only(editor_resource_id(resource_id));
		}
	}
}
