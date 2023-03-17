#pragma once
#include "application/setups/editor/commands/edit_project_settings_command.h"

void edit_project_settings_command::undo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	in.setup.project.settings = before;

	if (do_inspector) {
		in.setup.inspect_project_settings();
		in.setup.set_inspector_tab(tab);
	}
}

void edit_project_settings_command::redo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	before = in.setup.project.settings;
	in.setup.project.settings = after;

	if (do_inspector) {
		in.setup.inspect_project_settings();
		in.setup.set_inspector_tab(tab);
	}
}
