#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/project/editor_project_settings.h"
#include "application/setups/editor/gui/inspected_project_tab_type.h"

struct edit_project_settings_command {
	using editable_type = editor_project_settings;

	editor_command_meta meta;

	editable_type before;
	editable_type after;
	inspected_project_tab_type tab = inspected_project_tab_type::ARENA_PROPERTIES;

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}
};
