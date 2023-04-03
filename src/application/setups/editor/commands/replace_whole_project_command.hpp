#pragma once
#include "application/setups/editor/commands/replace_whole_project_command.h"

void replace_whole_project_command::undo(editor_command_input in) {
	in.setup.project = *before;
}

void replace_whole_project_command::redo(editor_command_input in) {
	in.setup.project = *after;
}
