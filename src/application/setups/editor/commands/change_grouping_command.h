#pragma once
#include <vector>
#include <string>

#include "game/transcendental/entity_id.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/editor_command_input.h"

struct change_grouping_command {
	// GEN INTROSPECTOR struct change_grouping_command
	editor_command_common common;
	std::vector<entity_id> affected_entities;
	std::vector<unsigned> group_indices_before;
	std::string built_description;
	bool create_new_group = false;
	// END GEN INTROSPECTOR

	void push_entry(entity_id);

	void redo(editor_command_input);
	void undo(editor_command_input);
	std::string describe() const;

	std::size_t size() const;
	bool empty() const;
};
