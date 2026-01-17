#pragma once
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/difficulty_type.h"

/*
	Alertness logic temporarily disabled for absolute reaction time.
	This will be revisited later to add reaction delays.
*/

inline bool update_alertness(
	arena_mode_ai_state&,
	const bool sees_target,
	const float,
	const difficulty_type
) {
	return sees_target;
}
