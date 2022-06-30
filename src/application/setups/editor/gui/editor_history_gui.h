#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

class editor_setup;
struct editor_history;

struct editor_history_gui_input {
	editor_setup& setup; 
	editor_history& history;
};

struct editor_history_gui : standard_window_mixin<editor_history_gui> {
	using base = standard_window_mixin<editor_history_gui>;
	using base::base;
	using introspect_base = base;

	bool scroll_to_latest_once = false;
	bool scroll_to_current_once = false;
	void perform(editor_history_gui_input);
};
