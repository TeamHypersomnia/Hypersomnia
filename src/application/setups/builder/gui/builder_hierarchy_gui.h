#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

struct builder_hierarchy_input {

};

struct builder_hierarchy_gui : standard_window_mixin<builder_hierarchy_gui> {
	using base = standard_window_mixin<builder_hierarchy_gui>;
	using base::base;
	using introspect_base = base;

	void perform(builder_hierarchy_input);
};

