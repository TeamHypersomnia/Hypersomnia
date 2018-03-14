#pragma once
#include "augs/templates/type_in_list_id.h"

#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_flavour_id.h"
#include "application/setups/editor/editor_command_structs.h"

using invariant_id_type = type_in_list_id<invariant_list_t<type_list>>;
using component_id_type = type_in_list_id<invariant_list_t<type_list>>;

struct flavour_property_id {
	entity_flavour_id flavour_id;
	invariant_id_type invariant_id;
	edited_field_id field_id;

	bool operator==(const flavour_property_id& b) {
		return 
			flavour_id == b.flavour_id 
			&& invariant_id == b.invariant_id 
			&& field_id == b.field_id
		;
	}
};

struct change_flavour_property_command {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct change_flavour_property_command
	editor_command_common common;
	flavour_property_id property_id;

	std::vector<std::byte> value_before_change;
	std::vector<std::byte> value_after_change;
	// END GEN INTROSPECTOR

	void redo(editor_command_input) const;
	void undo(editor_command_input) const;
};
