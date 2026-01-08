#pragma once
#include "game/debug_drawing_settings.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/arena_mode_ai_structs.h"

template <class SetMovementTarget>
inline vec2 handle_random_movement(
	const ai_character_context& ctx,
	const bool pause_chase,
	SetMovementTarget set_movement_target
) {
	const auto distance_to_target = (ctx.ai_state.random_movement_target - ctx.character_pos).length();

	if (DEBUG_DRAWING.draw_ai_info) {
		DEBUG_LOGIC_STEP_LINES.emplace_back(pause_chase ? cyan : white, ctx.character_pos, ctx.ai_state.random_movement_target);
	}

	if (distance_to_target < 140.0f) {
		ctx.ai_state.movement_timer_remaining = 0.0f;
	}

	return set_movement_target(ctx.ai_state.random_movement_target, false, false);
}
