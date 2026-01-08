#pragma once
#include "game/cosmos/logic_step.h"
#include "game/messages/sound_cue_message.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/enums/filters.h"

/*
	Check for sound cues (footsteps) from enemy players and update target tracking.
*/

inline void listen_for_footsteps(
	const ai_character_context& ctx,
	const logic_step step,
	const bool is_ffa
) {
	const auto& sound_cues = step.get_queue<messages::sound_cue_message>();
	const auto bot_faction = ctx.character_handle.get_official_faction();
	const auto filter = predefined_queries::line_of_sight();

	auto is_enemy_faction = [&](const faction_type source_faction) {
		if (is_ffa) {
			return bot_faction != source_faction;
		}

		return bot_faction != source_faction && source_faction != faction_type::SPECTATOR;
	};

	auto current_target_distance = [&]() {
		if (ctx.ai_state.last_seen_target.is_set()) {
			return (ctx.ai_state.last_target_position - ctx.character_pos).length();
		}

		return std::numeric_limits<float>::max();
	}();

	for (const auto& cue : sound_cues) {
		if (!cue.source_entity.is_set()) {
			continue;
		}

		if (cue.source_entity == ctx.character_handle.get_id()) {
			continue;
		}

		const auto source = ctx.cosm[cue.source_entity];

		if (!source.alive()) {
			continue;
		}

		const auto source_faction = source.get_official_faction();

		if (!is_enemy_faction(source_faction)) {
			continue;
		}

		/*
			Check if within hearing range.
		*/
		const auto dist = (cue.position - ctx.character_pos).length();

		if (dist > cue.max_distance) {
			continue;
		}

		/*
			Update target if this enemy is closer than current target.
		*/
		if (dist < current_target_distance) {
			ctx.ai_state.last_seen_target = cue.source_entity;
			ctx.ai_state.chase_remaining_time = 5.0f;
			ctx.ai_state.last_target_position = cue.position;
			ctx.ai_state.has_dashed_for_last_seen_target = false;
			current_target_distance = dist;
		}
	}
}
