#pragma once
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/gui/standard_window_mixin.h"
#include "application/setups/editor/gui/asset_browser_settings.h"

struct editor_images_gui : standard_window_mixin<editor_images_gui> {
	using base = standard_window_mixin<editor_images_gui>;
	using base::base;

	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_images_gui
	asset_browser_settings browser_settings;
	// END GEN INTROSPECTOR

	bool acquire_missing_paths = true;

	void perform(editor_command_input);
};
