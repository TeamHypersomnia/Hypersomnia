#pragma once
#include "game/cosmos/logic_step.h"
#include "game/messages/sound_cue_message.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/enums/filters.h"
#include "game/modes/ai/intents/should_acquire_target_by_hearing.hpp"
#include "game/modes/ai/tasks/can_weapon_penetrate.hpp"
#include "augs/misc/randomization.h"

/*
	Check for sound cues (footsteps) from enemy players and update target tracking.
	
	Uses the new ai_target_tracking system instead of the old fields.
	
	Now includes:
	- LoS check: if we have line of sight to the heard enemy (regardless of angle), full_acquire
	- Penetration check: if we can shoot through obstacles to the target, full_acquire
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
			No direct line of sight. Check if we can penetrate walls to shoot them.
			If weapon can penetrate to the target position, full_acquire.
		*/
		if (::can_weapon_penetrate(ctx.character_handle, cue.position)) {
			ctx.ai_state.combat_target.full_acquire(
				rng,
				global_time_secs,
				cue.source_entity,
				cue.position
			);
			continue;
		}

		/*
			Can't see and can't penetrate. Check if faction-specific hearing aggro should kick in.
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
