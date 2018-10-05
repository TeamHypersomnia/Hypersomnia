#pragma once
#include <vector>
#include <string>

#include "game/cosmos/entity_id.h"
#include "application/setups/editor/commands/change_property_command.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/editor_command_input.h"

struct change_grouping_command {
	// GEN INTROSPECTOR struct change_grouping_command
	editor_command_common common;
	std::vector<entity_id> affected_entities;
	std::vector<unsigned> group_indices_before;
	std::vector<unsigned> group_indices_after;
	std::string built_description;
	bool all_to_new_group = false;
	// END GEN INTROSPECTOR

	void push_entry(entity_id);

	void redo(editor_command_input);
	void undo(editor_command_input);
	std::string describe() const;

	std::size_t size() const;
	bool empty() const;

	void sanitize(editor_command_input);
};

struct change_group_property_command : change_property_command<change_group_property_command> {
	using introspect_base = change_property_command<change_group_property_command>;

	// GEN INTROSPECTOR struct change_group_property_command
	unsigned group_index = static_cast<unsigned>(-1);
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return 1u;
	}
};
