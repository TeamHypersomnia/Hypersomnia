#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

struct editor_inspector_input {

};

struct editor_inspector_gui : standard_window_mixin<editor_inspector_gui> {
	using base = standard_window_mixin<editor_inspector_gui>;
	using base::base;
	using introspect_base = base;

	void perform(editor_inspector_input);
};

