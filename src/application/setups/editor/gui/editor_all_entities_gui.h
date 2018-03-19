#pragma once
#include "application/setups/editor/editor_command_structs.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

struct editor_all_entities_gui {
	// GEN INTROSPECTOR struct editor_all_entities_gui
	bool show = false;
	// END GEN INTROSPECTOR

	editor_all_entities_gui(const std::string& title) : title(title) {}

	entity_guid hovered_guid;

	void open();
	void perform(
		const std::unordered_set<entity_id>& only_match_entities,
	   	editor_command_input
	);

	void interrupt_tweakers();

private:
	std::string title;
	property_editor_gui properties_gui;
	bool acquire_once = false;
};
