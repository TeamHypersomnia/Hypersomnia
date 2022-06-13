#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

struct editor_layers_input {

};

struct editor_layers_gui : standard_window_mixin<editor_layers_gui> {
	using base = standard_window_mixin<editor_layers_gui>;
	using base::base;
	using introspect_base = base;

	void perform(editor_layers_input);
};

