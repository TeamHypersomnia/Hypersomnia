#pragma once
#include "augs/math/vec2.h"
#include "game/enums/faction_type.h"

struct arena_playtesting_context {
	// GEN INTROSPECTOR struct arena_playtesting_context
	vec2 initial_spawn_pos;
	faction_type first_player_faction = faction_type::SPECTATOR;
	// END GEN INTROSPECTOR

	bool operator==(const arena_playtesting_context&) const = default;
};
