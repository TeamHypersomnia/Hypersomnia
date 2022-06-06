#pragma once
#include "application/setups/debugger/commands/debugger_command_structs.h"
#include "application/setups/debugger/commands/change_property_command.h"

#include "application/setups/debugger/detail/field_address.h"

struct change_common_state_command : change_property_command<change_common_state_command> {
	friend augs::introspection_access;

	using introspect_base = change_property_command<change_common_state_command>;

	// GEN INTROSPECTOR struct change_common_state_command
	cosmos_common_field_address field;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return 1u;
	}
};
