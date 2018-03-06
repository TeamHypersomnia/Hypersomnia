#pragma once

struct editor_command_input;

struct editor_history_gui {
	bool show = false;

	void perform(editor_command_input);
};
