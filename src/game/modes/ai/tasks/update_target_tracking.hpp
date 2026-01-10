#pragma once
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"

inline void update_target_tracking(
	const ai_character_context& ctx,
	const entity_id closest_enemy
) {
	const auto target_pos = ctx.cosm[closest_enemy].get_logic_transform().pos;
	ctx.ai_state.last_seen_target = closest_enemy;
	ctx.ai_state.chase_remaining_time = 5.0f;
	ctx.ai_state.last_target_position = target_pos;
	ctx.ai_state.has_dashed_for_last_seen_target = false;
}
