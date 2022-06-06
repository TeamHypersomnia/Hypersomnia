#pragma once
#include <unordered_set>
#include "augs/misc/imgui/standard_window_mixin.h"

struct entity_id;
class debugger_setup;

struct debugger_summary_gui : standard_window_mixin<debugger_summary_gui> {
	using base = standard_window_mixin<debugger_summary_gui>;
	using base::base;
	using introspect_base = base;

	void perform(debugger_setup&);
};

struct debugger_coordinates_gui : standard_window_mixin<debugger_coordinates_gui> {
	using base = standard_window_mixin<debugger_coordinates_gui>;
	using base::base;
	using introspect_base = base;

	void perform(
		debugger_setup&,
		vec2i screen_size,
		vec2i mouse_pos,
		const std::unordered_set<entity_id>&
	);
};
