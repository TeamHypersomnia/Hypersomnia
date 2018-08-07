#pragma once
#include <vector>
#include <string>

#include "game/cosmos/entity_id.h"
#include "application/setups/editor/commands/change_property_command.h"
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/editor_command_input.h"

struct change_current_mode_property_command : change_property_command<change_current_mode_property_command> {
	using introspect_base = change_property_command<change_current_mode_property_command>;

	// GEN INTROSPECTOR struct change_current_mode_property_command
	field_address field;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return 1u;
	}
};
