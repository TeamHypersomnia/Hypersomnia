#pragma once
#include "application/setups/editor/editor_command_structs.h"

struct editor_all_entities_gui {
	// GEN INTROSPECTOR struct editor_all_entities_gui
	bool show = false;
	// END GEN INTROSPECTOR

	void open();
	void perform(editor_command_input);

private:
	bool acquire_once = false;
};
