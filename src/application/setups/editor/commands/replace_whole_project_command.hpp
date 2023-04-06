#pragma once
#include "application/setups/editor/commands/replace_whole_project_command.h"

void replace_whole_project_command::undo(editor_command_input in) {
	{
		/*
			We need to do this.
			That is because the original "after" is an autosave loaded directly from HDD.

			If we assigned it later on redo, and then rescanned for new resources on HDD again,
			they could potentially create different set of resource ids,
			therefore invalidating later commands in editor history.
		*/

		*after = in.setup.get_project();
	}

	const bool undoing_to_first_revision = true;
	in.setup.assign_project(*before, undoing_to_first_revision);
}

void replace_whole_project_command::redo(editor_command_input in) {
	const bool undoing_to_first_revision = false;
	in.setup.assign_project(*after, undoing_to_first_revision);
}
