#pragma once

struct editor_command_input;

struct editor_history_gui {
	bool show = false;
	bool acquire_once = false;

	void open();
	void perform(editor_command_input);
};
