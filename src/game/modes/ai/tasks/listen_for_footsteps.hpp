#pragma once
#include "game/cosmos/logic_step.h"
#include "game/messages/sound_cue_message.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/enums/filters.h"
#include "game/modes/ai/intents/should_acquire_target_by_hearing.hpp"
#include "augs/misc/randomization.h"

/*
	Check for sound cues (footsteps) from enemy players and update target tracking.
	
	Uses the new ai_target_tracking system instead of the old fields.
	
	Now includes:
	- LoS check: if we have line of sight to the heard enemy (regardless of angle), full_acquire
	- should_acquire_target_by_hearing: faction-specific logic for aggressive acquisition
*/

inline void listen_for_footsteps(
	const ai_character_context& ctx,
	const logic_step step,
	const bool is_ffa,
	const real32 global_time_secs,
	const bool bomb_planted
) {
	const auto& sound_cues = step.get_queue<messages::sound_cue_message>();
	const auto bot_faction = ctx.character_handle.get_official_faction();
	const auto filter = predefined_queries::line_of_sight();

	auto rng_state = xorshift_state{ static_cast<uint64_t>(global_time_secs * 1000.0f + 54321) };
	auto rng = randomization(rng_state);

	auto is_enemy_faction = [&](const faction_type source_faction) {
		if (is_ffa) {
			return bot_faction != source_faction;
		}

		return bot_faction != source_faction && source_faction != faction_type::SPECTATOR;
	};

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
			Check if we have line of sight to the heard enemy (regardless of looking angle).
			If so, full_acquire the target.
		*/
		const auto raycast = ctx.physics.ray_cast_px(
			ctx.cosm.get_si(),
			ctx.character_pos,
			cue.position,
			filter,
			ctx.character_handle
		);

		if (!raycast.hit) {
			/*
				Clear line of sight to heard enemy - fully acquire target.
			*/
			ctx.ai_state.combat_target.full_acquire(
				rng,
				global_time_secs,
				cue.source_entity,
				cue.position
			);
			continue;
		}

		/*
			No line of sight. Check if faction-specific hearing aggro should kick in.
		*/
		if (::should_acquire_target_by_hearing(bot_faction, bomb_planted)) {
			ctx.ai_state.combat_target.full_acquire(
				rng,
				global_time_secs,
				cue.source_entity,
				cue.position
			);
			continue;
		}

		/*
			Default: Update target tracking via acquire_target_heard.
			This updates last_known_pos if it matches current target,
			otherwise just notes the position.
		*/
		ctx.ai_state.combat_target.acquire_target_heard(
			global_time_secs,
			cue.source_entity,
			cue.position
		);
	}
}
