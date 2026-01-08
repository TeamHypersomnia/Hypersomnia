#pragma once
#include "game/debug_drawing_settings.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/arena_mode_ai_structs.h"

template <class SetMovementTarget>
inline vec2 handle_active_chase(
	const ai_character_context& ctx,
	const bool sees_target,
	const float dt_secs,
	SetMovementTarget set_movement_target
) {
	const auto distance_to_last_seen = (ctx.ai_state.last_target_position - ctx.character_pos).length();
	const auto reached_threshold = 20.0f;

	ctx.ai_state.chase_remaining_time -= dt_secs;

	if (distance_to_last_seen < reached_threshold && !sees_target) {
		ctx.ai_state.chase_remaining_time = 0.0f;
	}

	if (ctx.ai_state.chase_remaining_time > 0.0f) {
		if (sees_target) {
			if (distance_to_last_seen < 250.0f) {
				if (ctx.ai_state.chase_timeout < 0.0f) {
					ctx.ai_state.movement_timer_remaining = 0.0f;
				}

				ctx.ai_state.chase_timeout = 1.0f;
				const auto dir = set_movement_target(ctx.ai_state.random_movement_target, false, false);

				if (DEBUG_DRAWING.draw_ai_info) {
					DEBUG_LOGIC_STEP_LINES.emplace_back(yellow, ctx.character_pos, ctx.ai_state.last_target_position);
				}

				return dir;
			}
			else {
				const auto dir = set_movement_target(ctx.ai_state.last_target_position, false, false);

				if (DEBUG_DRAWING.draw_ai_info) {
					DEBUG_LOGIC_STEP_LINES.emplace_back(yellow, ctx.character_pos, ctx.ai_state.last_target_position);
				}

				return dir;
			}
		}
		else {
			const auto should_dash = distance_to_last_seen < 400.0f && !ctx.ai_state.has_dashed_for_last_seen_target;

			if (should_dash) {
				ctx.ai_state.has_dashed_for_last_seen_target = true;
			}

			const auto dir = set_movement_target(ctx.ai_state.last_target_position, true, should_dash);

			if (DEBUG_DRAWING.draw_ai_info) {
				DEBUG_LOGIC_STEP_LINES.emplace_back(orange, ctx.character_pos, ctx.ai_state.last_target_position);
			}

			return dir;
		}
	}
	else {
		ctx.ai_state.last_seen_target = entity_id::dead();
		ctx.ai_state.chase_remaining_time = 0.0f;
		return set_movement_target(ctx.ai_state.random_movement_target, false, false);
	}
}
