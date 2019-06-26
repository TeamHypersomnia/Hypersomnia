#pragma once
#include "game/enums/game_intent_type.h"
#include "augs/math/vec2.h"

raw_game_motion to_game_motion(
	raw_game_motion motion,
	vec2 simulated_offset,
	const vec2& sensitivity,
	vec2i screen_size
);
