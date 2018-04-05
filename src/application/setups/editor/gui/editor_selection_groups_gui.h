#pragma once
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

struct editor_settings;
struct editor_command_input;

struct editor_selection_groups_gui {
	// GEN INTROSPECTOR struct editor_selection_groups_gui
	bool show = false;
	// END GEN INTROSPECTOR

	void open();
	void perform(bool has_ctrl, editor_command_input);

private:
	unsigned currently_renaming = -1;
};
