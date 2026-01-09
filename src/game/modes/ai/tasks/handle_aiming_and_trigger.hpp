#pragma once
#include "game/components/sentience_component.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"

inline void handle_aiming_and_trigger(
	const ai_character_context& ctx,
	const bool has_target,
	const entity_id closest_enemy
) {
	bool trigger = false;

	if (has_target) {
		const auto target_pos = ctx.cosm[closest_enemy].get_logic_transform().pos;
		const auto aim_direction = target_pos - ctx.character_pos;
		ctx.ai_state.target_crosshair_offset = aim_direction;

		if (auto crosshair = ctx.character_handle.find_crosshair()) {
			const auto current_aim = vec2(crosshair->base_offset).normalize();
			const auto target_aim = vec2(aim_direction).normalize();
			const auto angle_diff = current_aim.degrees_between(target_aim);

			trigger = angle_diff <= 25.0f;
		}
	}

	if (ctx.character_handle.is_frozen()) {
		trigger = false;
	}

	if (auto sentience = ctx.character_handle.find<components::sentience>()) {
		sentience->hand_flags[0] = sentience->hand_flags[1] = trigger;
	}
}
