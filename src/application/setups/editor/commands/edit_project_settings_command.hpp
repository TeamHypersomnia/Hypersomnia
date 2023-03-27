#pragma once
#include "application/setups/editor/commands/edit_project_settings_command.h"

void edit_project_settings_command::undo(editor_command_input in) {
	const bool do_inspector = do_inspector_at_all && !in.skip_inspector;

	std::visit([&]<typename T>(T& typed_before) {
		auto& current = in.setup.project.template get<T>();
		current = typed_before;
	}, before);

	if (do_inspector) {
		in.setup.inspect_project_settings();
		in.setup.set_inspector_tab(tab);
	}
}

void edit_project_settings_command::redo(editor_command_input in) {
	const bool do_inspector = do_inspector_at_all && !in.skip_inspector;

	std::visit([&]<typename T>(T& typed_after) {
		auto& current = in.setup.project.template get<T>();

		before = current;
		current = typed_after;
	}, after);

	if (do_inspector) {
		in.setup.inspect_project_settings();
		in.setup.set_inspector_tab(tab);
	}
}
