#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"
#include "augs/misc/randomization.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/cosmos/entity_handle.h"

/*
	Combat time configuration for bomb game mode AI.
	All values in seconds.
*/
namespace ai_combat_time {
	constexpr auto BOMB_CARRIER_MIN_SECS = 0.5f;
	constexpr auto BOMB_CARRIER_MAX_SECS = 2.0f;

	constexpr auto DEFUSER_LONG_ENGAGEMENT_MIN_SECS = 2.0f;
	constexpr auto DEFUSER_LONG_ENGAGEMENT_MAX_SECS = 12.0f;
	constexpr auto DEFUSER_SHORT_ENGAGEMENT_MIN_SECS = 0.5f;
	constexpr auto DEFUSER_SHORT_ENGAGEMENT_MAX_SECS = 0.8f;
	constexpr auto DEFUSER_ENGAGEMENT_THRESHOLD_SECS = 20.0f;

	constexpr auto DEFAULT_MIN_SECS = 5.0f;
	constexpr auto DEFAULT_MAX_SECS = 10.0f;
}

inline real32 pick_combat_time_secs(
	randomization& rng,
	const bool is_bomb_carrier,
	const bool is_defuser,
	const real32 bomb_time_remaining_secs
) {
	using namespace ai_combat_time;

	if (is_bomb_carrier) {
		return rng.randval(BOMB_CARRIER_MIN_SECS, BOMB_CARRIER_MAX_SECS);
	}

	if (is_defuser) {
		if (bomb_time_remaining_secs >= DEFUSER_ENGAGEMENT_THRESHOLD_SECS) {
			return rng.randval(DEFUSER_LONG_ENGAGEMENT_MIN_SECS, DEFUSER_LONG_ENGAGEMENT_MAX_SECS);
		}

		return rng.randval(DEFUSER_SHORT_ENGAGEMENT_MIN_SECS, DEFUSER_SHORT_ENGAGEMENT_MAX_SECS);
	}

	return rng.randval(DEFAULT_MIN_SECS, DEFAULT_MAX_SECS);
}

/*
	Persistent combat target tracking.
	
	This struct tracks facts about a combat target independently of behavior state.
	It is updated before evaluating the behavior tree, possibly in post_solve.
	
	Whether to enter COMBAT state is decided in ::eval_behavior_tree based on
	whether combat_target.active() returns true.
*/

struct ai_target_tracking {
	// GEN INTROSPECTOR struct ai_target_tracking
	entity_id id = entity_id::dead();
	vec2 last_visual_pos = vec2::zero;
	vec2 last_known_pos = vec2::zero;
	real32 last_known_time_secs = 0.0f;
	real32 engagement_started_secs = 0.0f;
	real32 engagement_timeout_secs = 0.0f;
	/*
		When true, the engagement timeout is measured from engagement_started_secs
		(set at first acquisition) rather than last_known_time_secs (updated on every
		footstep/sight refresh). All Metropolis bots use this while the bomb is planted
		so footstep spam cannot extend their engagement indefinitely and they prioritise
		reaching the bomb over prolonged combat.
	*/
	bool timeout_from_engagement_start = false;
	// END GEN INTROSPECTOR

	/*
		Returns true if we have an active combat target.
		A target is active if:
		- We have a valid entity id
		- We've seen or heard them within the chosen combat timeout
	*/
	bool within_engagement_window(const cosmos& cosm, const real32 global_time_secs) const {
		if (const auto handle = cosm[id]) {
			if (sentient_and_conscious(handle)) {
				const auto reference = timeout_from_engagement_start ? engagement_started_secs : last_known_time_secs;
				const auto elapsed = global_time_secs - reference;
				return elapsed <= engagement_timeout_secs;
			}
		}

		return false;
	}

	/*
		Acquire a seen enemy target.
		Updates all tracking fields.
		
		If the new target is different and closer, switch to it.
	*/
	void on_visual_contact(
		randomization& rng,
		const real32 global_time_secs,
		const entity_id enemy,
		const vec2 enemy_pos,
		const vec2 bot_pos,
		const bool is_bomb_carrier = false,
		const bool is_defuser = false,
		const real32 bomb_time_remaining_secs = 1000.0f,
		const real32 reaction_time_secs = 0.0f
	) {
		const auto dist_to_new = (enemy_pos - bot_pos).length();

		if (id != enemy && id.is_set()) {
			/*
				Already tracking a different target.
				Only switch if the new one is closer.
			*/
			const auto dist_to_current = (last_known_pos - bot_pos).length();

			if (dist_to_new >= dist_to_current) {
				/*
					Current target is closer or equal - don't switch.
					But still update last_known if it's the same target.
				*/
				return;
			}
		}

		/*
			Either no current target, or new target is closer.
			Switch to the new target.
		*/
		const bool switching_target = (id != enemy);

		id = enemy;
		last_visual_pos = enemy_pos;
		last_known_pos = enemy_pos;
		last_known_time_secs = global_time_secs;

		/*
			Reset engagement timer if:
			- Switching to a different target, or
			- Re-acquiring the same target after engagement timeout expired
		*/
		const bool engagement_timed_out = (global_time_secs - engagement_started_secs) > engagement_timeout_secs;
		
		if (switching_target || engagement_timed_out) {
			/*
				Re-randomize combat time when switching targets or re-engaging after timeout.
			*/
			engagement_started_secs = global_time_secs;
			engagement_timeout_secs = ::pick_combat_time_secs(rng, is_bomb_carrier, is_defuser, bomb_time_remaining_secs) + reaction_time_secs;
			timeout_from_engagement_start = is_defuser || is_bomb_carrier;
		}
	}

	/*
		Acquire a heard enemy position.
		Only updates last_known_pos if the target matches,
		does not change target or reset combat time.
	*/
	void on_heard_footstep(
		const real32 global_time_secs,
		const entity_id enemy,
		const vec2 heard_pos
	) {
		if (id == enemy) {
			last_known_pos = heard_pos;
			last_known_time_secs = global_time_secs;
		}
		else if (!id.is_set()) {
			/*
				No current target - use heard position as starting point.
				Don't set id yet - we need to see them first for full combat.
			*/
			last_known_pos = heard_pos;
		}
	}

	/*
		Force acquire a target (e.g., from damage).
		Always acquires regardless of distance.
	*/
	void force_engage(
		randomization& rng,
		const real32 global_time_secs,
		const entity_id enemy,
		const vec2 enemy_pos,
		const bool is_bomb_carrier = false,
		const bool is_defuser = false,
		const real32 bomb_time_remaining_secs = 1000.0f,
		const real32 reaction_time_secs = 0.0f
	) {
		id = enemy;
		last_visual_pos = enemy_pos;
		last_known_pos = enemy_pos;
		last_known_time_secs = global_time_secs;
		engagement_started_secs = global_time_secs;
		engagement_timeout_secs = ::pick_combat_time_secs(rng, is_bomb_carrier, is_defuser, bomb_time_remaining_secs) + reaction_time_secs;
		timeout_from_engagement_start = is_defuser || is_bomb_carrier;
	}

	/*
		Clear the tracking state.
	*/
	void clear() {
		id = entity_id::dead();
		last_visual_pos = vec2::zero;
		last_known_pos = vec2::zero;
		last_known_time_secs = 0.0f;
		engagement_started_secs = 0.0f;
		engagement_timeout_secs = 0.0f;
		timeout_from_engagement_start = false;
	}
};

