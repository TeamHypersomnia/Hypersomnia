#pragma once
#include "augs/misc/imgui/path_tree_structs.h"

struct asset_path_browser_settings {
	// GEN INTROSPECTOR struct asset_path_browser_settings
	path_tree_settings tree_settings;
	bool show_orphaned = false;
	bool show_using_locations = false;
	bool show_properties_column = true;
	// END GEN INTROSPECTOR

	void do_tweakers();
};
