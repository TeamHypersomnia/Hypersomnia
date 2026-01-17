#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/entity_id.h"

struct arena_playtesting_context {
	// GEN INTROSPECTOR struct arena_playtesting_context
	vec2 initial_spawn_pos;
	faction_type first_player_faction = faction_type::SPECTATOR;

	std::optional<transformr> debug_pathfinding_end;
	entity_id debug_pathfinding_bomb_target;
	// END GEN INTROSPECTOR

	bool operator==(const arena_playtesting_context&) const = default;
};
