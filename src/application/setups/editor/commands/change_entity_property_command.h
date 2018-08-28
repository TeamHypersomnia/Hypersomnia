#pragma once
#include "game/organization/all_components_declaration.h"

#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/commands/change_property_command.h"

#include "application/setups/editor/detail/field_address.h"

struct entity_property_id {
	unsigned component_id = static_cast<unsigned>(-1);
	entity_field_address field;
};

using affected_entities_type = std::vector<entity_id_base>;

struct change_entity_property_command : change_property_command<change_entity_property_command> {
	using introspect_base = change_property_command<change_entity_property_command>;

	// GEN INTROSPECTOR struct change_entity_property_command
	entity_type_id type_id;
	affected_entities_type affected_entities;
	entity_property_id property_id;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return affected_entities.size();
	}
};
