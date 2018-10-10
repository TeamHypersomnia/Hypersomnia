#pragma once
#include <vector>
#include <string>

#include "game/cosmos/entity_id.h"
#include "game/modes/all_mode_includes.h"
#include "application/setups/editor/commands/change_property_command.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/detail/field_address.h"

struct change_mode_vars_property_command : change_property_command<change_mode_vars_property_command> {
	using introspect_base = change_property_command<change_mode_vars_property_command>;

	// GEN INTROSPECTOR struct change_mode_vars_property_command
	mode_field_address field;
	raw_mode_vars_id vars_id;
	mode_type_id vars_type_id;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		/* TODO: Implement multiple selection for mode vars */
		return 1u;
	}
};

struct change_current_mode_property_command : change_property_command<change_current_mode_property_command> {
	using introspect_base = change_property_command<change_current_mode_property_command>;

	// GEN INTROSPECTOR struct change_current_mode_property_command
	mode_field_address field;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return 1u;
	}
};
