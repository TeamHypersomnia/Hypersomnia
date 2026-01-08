#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "game/enums/faction_type.h"

struct arena_playtesting_context {
	// GEN INTROSPECTOR struct arena_playtesting_context
	vec2 initial_spawn_pos;
	faction_type first_player_faction = faction_type::SPECTATOR;

	/*
		For pathfinding debug testing.
	*/
	std::optional<vec2> debug_pathfinding_end;
	bool debug_pathfinding_to_bomb = false;
	// END GEN INTROSPECTOR

	bool operator==(const arena_playtesting_context&) const = default;
};
