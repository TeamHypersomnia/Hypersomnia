#pragma once
#include "augs/misc/imgui/path_tree_settings.h"

struct asset_browser_settings {
	// GEN INTROSPECTOR struct asset_browser_settings
	path_tree_settings tree_settings;
	bool show_unused = false;
	// END GEN INTROSPECTOR

	void do_tweakers();
};
