#pragma once
#include <vector>
#include <variant>
#include "augs/readwrite/memory_stream.h"
#include "game/transcendental/entity_id.h"

struct changed_field_record {
	// GEN INTROSPECTOR struct changed_field_record
	unsigned changed_value_index = 0;

	std::vector<std::byte> new_value;
	std::vector<std::byte> old_value;
	// END GEN INTROSPECTOR
};

struct change_component_command {
	// GEN INTROSPECTOR struct change_component_command
	unsigned component_index = static_cast<unsigned>(-1);
	changed_field_record record;
	// END GEN INTROSPECTOR
};

struct duplicate_entity_command {

};

struct delete_entity_command {
	// GEN INTROSPECTOR struct delete_entity_command
	std::vector<std::byte> deleted_content;
	// END GEN INTROSPECTOR
};

struct existing_entity_command {
	using op_type = std::variant<
		change_component_command,
		duplicate_entity_command,
		delete_entity_command
	>;

	// GEN INTROSPECTOR struct existing_entity_command
	std::vector<entity_id> entities;
	op_type operation;
	// END GEN INTROSPECTOR
};

struct change_common_state_command {
	// GEN INTROSPECTOR struct change_common_state_command 
	changed_field_record record;
	// END GEN INTROSPECTOR
};

using editor_command = std::variant<
	change_common_state_command,
	existing_entity_command
>;

struct editor_history {
	// GEN INTROSPECTOR struct editor_history
	unsigned current_index = 0;
	unsigned last_saved_at = 0;
	std::vector<editor_command> commands;
	// END GEN INTROSPECTOR
};