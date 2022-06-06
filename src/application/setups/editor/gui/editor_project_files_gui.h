#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

struct editor_project_files_input {

};

struct editor_project_files_gui : standard_window_mixin<editor_project_files_gui> {
	using base = standard_window_mixin<editor_project_files_gui>;
	using base::base;
	using introspect_base = base;

	void perform(editor_project_files_input);
};

