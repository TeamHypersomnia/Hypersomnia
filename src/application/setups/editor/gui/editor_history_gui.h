#pragma once

struct editor_command_input;

struct editor_history_gui {
	// GEN INTROSPECTOR struct editor_history_gui
	bool show = false;
	// END GEN INTROSPECTOR

	void open();
	void perform(editor_command_input);

private:
	bool acquire_once = false;
};
