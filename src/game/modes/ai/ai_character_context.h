#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_handle.h"

struct arena_mode_ai_state;

class cosmos;
class physics_world_cache;

struct ai_character_context {
	arena_mode_ai_state& ai_state;
	const vec2 character_pos;
	const physics_world_cache& physics;
	const cosmos& cosm;
	const entity_handle character_handle;
};
