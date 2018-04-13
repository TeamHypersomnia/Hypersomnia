#pragma once
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/gui/standard_window_mixin.h"

struct files_browser_settings {
	// GEN INTROSPECTOR struct files_browser_settings
	bool linear_view = false;
	bool prettify_filenames = true;
	// END GEN INTROSPECTOR

	void do_tweakers();
};

struct editor_images_gui : standard_window_mixin<editor_images_gui> {
	using base = standard_window_mixin<editor_images_gui>;
	using base::base;

	// GEN INTROSPECTOR struct editor_images_gui
	// INTROSPECT BASE standard_window_mixin<editor_images_gui>
	files_browser_settings browser_settings;
	// END GEN INTROSPECTOR

	void perform(editor_command_input);
};
