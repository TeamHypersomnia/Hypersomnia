#pragma once
#include "application/setups/debugger/commands/editor_command_structs.h"
#include "application/setups/debugger/property_editor/property_editor_structs.h"
#include "augs/misc/imgui/standard_window_mixin.h"

struct editor_settings;
struct editor_command_input;

struct editor_selection_groups_gui : standard_window_mixin<editor_selection_groups_gui> {
	using base = standard_window_mixin<editor_selection_groups_gui>;
	using base::base;
	using introspect_base = base;

	void perform(bool has_ctrl, editor_command_input);
};
