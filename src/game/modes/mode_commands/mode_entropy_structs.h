#pragma once
#include "game/common_state/entity_name_str.h"
#include "game/modes/mode_player_id.h"
#include "game/modes/mode_commands/match_command.h"

struct add_player_input {
	// GEN INTROSPECTOR struct add_player_input
	mode_player_id id;
	entity_name_str name;
	faction_type faction = faction_type::SPECTATOR;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return id.is_set();
	}

	bool operator==(const add_player_input& b) const {
		return 
			id == b.id
			&& name == b.name
			&& faction == b.faction
		;
	}
};

using all_general_mode_commands_variant = std::variant<
	std::monostate,
	match_command
>;	
