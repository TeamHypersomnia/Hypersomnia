#pragma once
#include "game/organization/all_entity_types_declaration.h"

#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/commands/id_allocating_command.h"
#include "game/transcendental/entity_flavour_id.h"

namespace augs {
	struct introspection_access;
}

struct create_flavour_command : id_allocating_command<raw_entity_flavour_id> {
	using base = id_allocating_command<raw_entity_flavour_id>;
	using introspect_base = base;

	// GEN INTROSPECTOR struct create_flavour_command
	entity_type_id type_id;
	std::string built_description;
	// END GEN INTROSPECTOR

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

struct duplicate_flavour_command {
	// GEN INTROSPECTOR struct duplicate_flavour_command
	create_flavour_command subcommand;
	entity_flavour_id copy_from;
	// END GEN INTROSPECTOR

	std::string describe() const;
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

struct delete_flavour_command {

};
