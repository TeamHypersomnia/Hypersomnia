#pragma once

struct editor_folder;

struct editor_history_gui {
	bool show = false;

	void perform(editor_folder&);
};
