#pragma once
#include <unordered_map>
#include "application/setups/editor/gui/standard_window_mixin.h"
#include "augs/misc/enum/enum_array.h"

enum class editor_tutorial_type {
	// GEN INTROSPECTOR enum class editor_tutorial_type
	WELCOME,
	UNSAVED_CHANGES,
	EMPTY_PROJECT,
	FILLED_UNTITLED_PROJECT,
	MOVING_ENTITIES,
	ROTATING_ENTITIES,
	RESIZING_ENTITIES,
	SAVED_PROJECT,
	SELECTED_ENTITIES,

	COUNT
	// END GEN INTROSPECTOR
};

struct entity_id;
class editor_setup;

struct editor_tutorial_gui : standard_window_mixin<editor_tutorial_gui> {
	using base = standard_window_mixin<editor_tutorial_gui>;
	using base::base;
	using introspect_base = base;

	augs::enum_array<std::string, editor_tutorial_type> context_manuals;
	std::unordered_map<std::string, std::string> dialog_manuals;

	editor_tutorial_gui(const std::string&);

	void perform(const editor_setup&);
};

