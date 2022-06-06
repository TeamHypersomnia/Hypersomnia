#pragma once
#include <vector>
#include <string>

#include "game/cosmos/entity_id.h"
#include "game/modes/all_mode_includes.h"
#include "game/modes/mode_player_id.h"
#include "game/modes/ruleset_id.h"
#include "application/setups/debugger/commands/change_property_command.h"
#include "application/setups/debugger/commands/debugger_command_structs.h"
#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/detail/field_address.h"

struct change_ruleset_property_command : change_property_command<change_ruleset_property_command> {
	using introspect_base = change_property_command<change_ruleset_property_command>;

	// GEN INTROSPECTOR struct change_ruleset_property_command
	mode_field_address field;
	raw_ruleset_id id;
	mode_type_id type_id;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		/* TODO: Implement multiple selection for mode vars */
		return 1u;
	}
};

struct change_rulesets_meta_property : change_property_command<change_rulesets_meta_property> {
	using introspect_base = change_property_command<change_rulesets_meta_property>;

	// GEN INTROSPECTOR struct change_rulesets_meta_property
	only_trivial_field_address field;
	// END GEN INTROSPECTOR

	auto count_affected() const {
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

struct change_mode_player_property_command : change_property_command<change_mode_player_property_command> {
	using introspect_base = change_property_command<change_mode_player_property_command>;

	// GEN INTROSPECTOR struct change_mode_player_property_command
	mode_player_id player_id;
	mode_field_address field;
	// END GEN INTROSPECTOR

	auto count_affected() const {
		return 1u;
	}
};
