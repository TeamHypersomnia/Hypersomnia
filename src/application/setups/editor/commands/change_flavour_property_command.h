#pragma once
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_flavour_id.h"
#include "application/setups/editor/editor_command_structs.h"
#include "application/setups/editor/property_editor_structs.h"

struct flavour_property_id {
	entity_flavour_id flavour_id;
	unsigned invariant_id = static_cast<unsigned>(-1);
	unsigned field_offset = static_cast<unsigned>(-1);
	edited_field_type_id field_type;

	/* bool operator==(const flavour_property_id& b) { */
	/* 	return */ 
	/* 		flavour_id == b.flavour_id */ 
	/* 		&& invariant_id == b.invariant_id */ 
	/* 		&& field_offset == b.field_offset */
	/* 		&& field_type == b.field_type */
	/* 	; */
	/* } */
};

struct change_flavour_property_command {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct change_flavour_property_command
	editor_command_common common;
	flavour_property_id property_id;

	std::vector<std::byte> value_before_change;
	std::vector<std::byte> value_after_change;

	std::string built_description;
	// END GEN INTROSPECTOR

	void redo(editor_command_input);
	void undo(editor_command_input);

	void rewrite_change(
		std::vector<std::byte>&& new_value,
		editor_command_input
	);

	std::string describe() const;
};
