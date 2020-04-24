#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

struct builder_project_files_input {

};

struct builder_project_files_gui : standard_window_mixin<builder_project_files_gui> {
	using base = standard_window_mixin<builder_project_files_gui>;
	using base::base;
	using introspect_base = base;

	void perform(builder_project_files_input);
};

