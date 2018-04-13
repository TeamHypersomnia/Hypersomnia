#pragma once
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/gui/standard_window_mixin.h"

struct editor_settings;
struct editor_command_input;

struct editor_selection_groups_gui : standard_window_mixin<editor_selection_groups_gui> {
	using base = standard_window_mixin<editor_selection_groups_gui>;
	using base::base;

	void perform(bool has_ctrl, editor_command_input);

private:
	unsigned currently_renaming = -1;
};
