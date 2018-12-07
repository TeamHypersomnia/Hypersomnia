#pragma once
#include "game/common_state/entity_name_str.h"
#include "game/modes/mode_player_id.h"

struct add_player_input {
	// GEN INTROSPECTOR struct add_player_input
	mode_player_id id;
	entity_name_str name;
	faction_type faction = faction_type::SPECTATOR;
	// END GEN INTROSPECTOR
};

struct mode_restart_command {
	// GEN INTROSPECTOR struct mode_restart_command
	pad_bytes<1> pad;
	// END GEN INTROSPECTOR
};

using all_general_mode_commands_variant = std::variant<
	std::monostate,
	mode_restart_command
>;	
