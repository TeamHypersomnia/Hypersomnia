#pragma once
#include "game/enums/game_intent_type.h"
#include "augs/math/vec2.h"

void adjust_game_motions(
	vec2 simulated_offset,
	const vec2& sensitivity,
	vec2i screen_size,
	game_motions& motions
);
