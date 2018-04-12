#pragma once
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

struct editor_settings;
struct editor_command_input;

struct editor_common_state_gui {
	// GEN INTROSPECTOR struct editor_common_state_gui
	bool show = false;
	// END GEN INTROSPECTOR

	void open();
	void perform(const editor_settings&, editor_command_input);

private:
	property_editor_state properties_gui;
	bool acquire_once = false;
};
