#pragma once
#include "application/setups/editor/editor_command_structs.h"

struct editor_all_entities_gui {
	bool show = false;
	bool acquire_once = false;

	void open();
	void perform(editor_command_input);
};
