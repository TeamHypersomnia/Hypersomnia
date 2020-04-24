#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

struct builder_inspector_input {

};

struct builder_inspector_gui : standard_window_mixin<builder_inspector_gui> {
	using base = standard_window_mixin<builder_inspector_gui>;
	using base::base;
	using introspect_base = base;

	void perform(builder_inspector_input);
};

