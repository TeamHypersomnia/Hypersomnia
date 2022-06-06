#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

struct editor_hierarchy_input {

};

struct editor_hierarchy_gui : standard_window_mixin<editor_hierarchy_gui> {
	using base = standard_window_mixin<editor_hierarchy_gui>;
	using base::base;
	using introspect_base = base;

	void perform(editor_hierarchy_input);
};

