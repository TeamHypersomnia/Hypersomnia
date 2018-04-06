#pragma once
#include <unordered_set>

struct entity_id;
class editor_setup;

struct editor_summary_gui {
	// GEN INTROSPECTOR struct editor_summary_gui
	bool show = false;
	// END GEN INTROSPECTOR

	void open();
	void perform(editor_setup&);
};

struct editor_coordinates_gui {
	// GEN INTROSPECTOR struct editor_coordinates_gui
	bool show = false;
	// END GEN INTROSPECTOR

	void open();
	void perform(editor_setup&, vec2i screen_size, vec2i mouse_pos, const std::unordered_set<entity_id>&);
};
