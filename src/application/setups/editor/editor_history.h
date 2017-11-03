#pragma once
#include <vector>
#include <variant>
#include "augs/readwrite/memory_stream.h"
#include "game/transcendental/entity_id.h"

struct change_common_state_command {
	// GEN INTROSPECTOR struct change_common_state_command
	std::vector<std::byte> changed_value;
	unsigned changed_value_index = 0;
	// END GEN INTROSPECTOR
};

struct change_component_command {
	// GEN INTROSPECTOR struct change_component_command
	std::vector<std::byte> changed_value;
	unsigned changed_value_index = 0;
	// END GEN INTROSPECTOR
};

struct remove_command {

};

struct duplicate_command {

};

struct existing_entity_command {
	using op_type = std::variant<
		change_component_command,
		remove_command,
		duplicate_command
	>;

	// GEN INTROSPECTOR struct existing_entity_command
	std::vector<entity_id> entities;
	op_type operation;
	// END GEN INTROSPECTOR
};

using editor_command = std::variant<
	change_common_state_command,
	existing_entity_command
>;

struct editor_history {
	// GEN INTROSPECTOR struct editor_history
	unsigned current_index = 0;
	std::vector<editor_command> commands;
	// END GEN INTROSPECTOR
};