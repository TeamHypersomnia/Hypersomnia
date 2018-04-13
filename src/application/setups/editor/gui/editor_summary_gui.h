#pragma once
#include <unordered_set>
#include "application/setups/editor/gui/standard_window_mixin.h"

struct entity_id;
class editor_setup;

struct editor_summary_gui : standard_window_mixin<editor_summary_gui> {
	using base = standard_window_mixin<editor_summary_gui>;
	using base::base;

	void perform(editor_setup&);
};

struct editor_coordinates_gui : standard_window_mixin<editor_coordinates_gui> {
	using base = standard_window_mixin<editor_coordinates_gui>;
	using base::base;

	void perform(
		editor_setup&,
		vec2i screen_size,
		vec2i mouse_pos,
		const std::unordered_set<entity_id>&
	);
};
