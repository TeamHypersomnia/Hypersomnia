#pragma once
#include "augs/enums/callback_result.h"
#include "game/organization/all_components_declaration.h"
#include "game/cosmos/entity_flavour_id.h"

#include "application/setups/debugger/commands/debugger_command_structs.h"
#include "application/setups/debugger/commands/change_property_command.h"

#include "application/setups/debugger/commands/detail/flavour_property_id.h"
#include "application/setups/debugger/commands/detail/entity_property_id.h"

using affected_flavours_type = std::vector<raw_entity_flavour_id>;

struct change_flavour_property_command : change_property_command<change_flavour_property_command> {
	friend augs::introspection_access;

	using introspect_base = change_property_command<change_flavour_property_command>;

	// GEN INTROSPECTOR struct change_flavour_property_command
	entity_type_id type_id;
	affected_flavours_type affected_flavours;
	flavour_property_id property_id;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return affected_flavours.size();
	}
};

struct change_initial_component_property_command : change_property_command<change_initial_component_property_command> {
	friend augs::introspection_access;

	using introspect_base = change_property_command<change_initial_component_property_command>;

	// GEN INTROSPECTOR struct change_initial_component_property_command
	entity_type_id type_id;
	affected_flavours_type affected_flavours;
	entity_property_id property_id;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return affected_flavours.size();
	}
};
