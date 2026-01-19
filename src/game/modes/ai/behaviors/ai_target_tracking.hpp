#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"
#include "augs/misc/randomization.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/cosmos/entity_handle.h"

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
	vec2 last_seen_pos = vec2::zero;
	vec2 last_known_pos = vec2::zero;
	real32 when_last_known_secs = 0.0f;
	real32 chosen_combat_time_secs = 0.0f;
	// END GEN INTROSPECTOR

	/*
		Returns true if we have an active combat target.
		A target is active if:
		- We have a valid entity id
		- We've seen or heard them within the chosen combat timeout
	*/
	bool active(const cosmos& cosm, const real32 global_time_secs) const {
		if (const auto handle = cosm[id]) {
			if (sentient_and_conscious(handle)) {
				return false;
			}

			const auto elapsed = global_time_secs - when_last_known_secs;
			return elapsed <= chosen_combat_time_secs;
		}

		return false;
	}

	/*
		Acquire a seen enemy target.
		Updates all tracking fields.
		
		If the new target is different and closer, switch to it.
	*/
	void acquire_target_seen(
		randomization& rng,
		const real32 global_time_secs,
		const entity_id enemy,
		const vec2 enemy_pos,
		const vec2 bot_pos
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
		last_seen_pos = enemy_pos;
		last_known_pos = enemy_pos;
		when_last_known_secs = global_time_secs;

		if (switching_target) {
			/*
				Re-randomize combat time when switching targets.
			*/
			chosen_combat_time_secs = rng.randval(5.0f, 10.0f);
		}
	}

	/*
		Acquire a heard enemy position.
		Only updates last_known_pos if the target matches,
		does not change target or reset combat time.
	*/
	void acquire_target_heard(
		const real32 global_time_secs,
		const entity_id enemy,
		const vec2 heard_pos
	) {
		if (id == enemy) {
			last_known_pos = heard_pos;
			when_last_known_secs = global_time_secs;
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
	void force_acquire(
		randomization& rng,
		const real32 global_time_secs,
		const entity_id enemy,
		const vec2 enemy_pos
	) {
		id = enemy;
		last_seen_pos = enemy_pos;
		last_known_pos = enemy_pos;
		when_last_known_secs = global_time_secs;
		chosen_combat_time_secs = rng.randval(5.0f, 10.0f);
	}

	/*
		Clear the tracking state.
	*/
	void clear() {
		id = entity_id::dead();
		last_seen_pos = vec2::zero;
		last_known_pos = vec2::zero;
		when_last_known_secs = 0.0f;
		chosen_combat_time_secs = 0.0f;
	}
};
