#pragma once
#include "application/setups/editor/gui/editor_inspector_gui.h"
#include "application/setups/editor/commands/editor_command_meta.h"

struct inspect_command {
	editor_command_meta meta;

	std::vector<inspected_variant> to_inspect;
	std::vector<inspected_variant> inspected_before;

	std::string built_description;

	const auto& describe() const {
		return built_description;
	}

	void undo(editor_command_input in);
	void redo(editor_command_input in);
};

