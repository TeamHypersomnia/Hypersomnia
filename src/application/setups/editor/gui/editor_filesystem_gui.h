#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/necessary_resources.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"

class editor_setup;

struct editor_filesystem_node;

struct editor_project_files_input {
	editor_filesystem_node& files_root;
	const ad_hoc_in_atlas_map& ad_hoc_atlas;
	const necessary_images_in_atlas_map& necessary_images;
};

struct editor_filesystem_gui : standard_window_mixin<editor_filesystem_gui> {
	using base = standard_window_mixin<editor_filesystem_gui>;
	using base::base;
	using introspect_base = base;

	const editor_filesystem_node* dragged_resource = nullptr;

	void perform(editor_project_files_input);

	void clear_pointers() {
		dragged_resource = nullptr;
	}
};

