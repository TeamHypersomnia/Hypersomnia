#pragma once
#include "application/setups/editor/commands/editor_command_meta.h"

struct editor_project;

struct replace_whole_project_command {
	editor_command_meta meta;

	std::unique_ptr<editor_project> before;
	std::unique_ptr<editor_project> after;

	std::string built_description;

	void undo(editor_command_input in);
	void redo(editor_command_input in);

	const auto& describe() const {
		return built_description;
	}
};
