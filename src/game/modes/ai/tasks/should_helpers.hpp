#pragma once
#include "game/modes/ai/arena_mode_ai_structs.h"

/*
	Determines if the bot should holster weapons.
	Returns true when patrolling/traveling to first waypoint without combat.
	Always false in COMBAT.
*/

inline bool should_holster_weapons(const arena_mode_ai_state& ai_state) {
	if (ai_state.current_state == bot_state_type::COMBAT) {
		return false;
	}

	if (ai_state.current_state == bot_state_type::PATROLLING) {
		if (ai_state.going_to_first_waypoint) {
			return true;
		}
	}

	if (ai_state.current_state == bot_state_type::PUSHING) {
		return true;
	}

	if (ai_state.current_state == bot_state_type::PLANTING) {
		return true;
	}

	return false;
}

/*
	Determines if the bot should sprint.
	- First/switching waypoint in patrol
	- Going to PUSH waypoint
	- In COMBAT (chasing)
*/

inline bool should_sprint(const arena_mode_ai_state& ai_state) {
	if (ai_state.current_state == bot_state_type::COMBAT) {
		return true;
	}

	if (ai_state.current_state == bot_state_type::PATROLLING && ai_state.going_to_first_waypoint) {
		return true;
	}

	if (ai_state.current_state == bot_state_type::PUSHING) {
		return true;
	}

	if (ai_state.current_state == bot_state_type::RETRIEVING_BOMB) {
		return true;
	}

	return false;
}

/*
	Determines if the bot should walk silently.
	Only when patrolling (and not going to first waypoint).
	85% chance to walk silently when choosing next waypoint.
*/

inline bool should_walk_silently(const arena_mode_ai_state& ai_state) {
	if (ai_state.current_state != bot_state_type::PATROLLING) {
		return false;
	}

	if (ai_state.going_to_first_waypoint) {
		return false;
	}

	return ai_state.walk_silently_to_next_waypoint;
}

/*
	Determines if the bot is currently camping on a waypoint.
*/

inline bool is_camping(const arena_mode_ai_state& ai_state) {
	return ai_state.current_state == bot_state_type::PATROLLING &&
	       ai_state.camp_timer > 0.0f &&
	       !ai_state.going_to_first_waypoint;
}

/*
	Check if we should dash when crossing last_seen_target_pos.
	Only applies in COMBAT state.
*/

inline bool should_dash_for_combat(
	arena_mode_ai_state& ai_state,
	const vec2 bot_pos
) {
	if (ai_state.current_state != bot_state_type::COMBAT) {
		return false;
	}

	if (ai_state.has_dashed_for_last_seen) {
		return false;
	}

	constexpr float DASH_RADIUS = 100.0f;
	const auto dist_to_last_seen = (bot_pos - ai_state.last_seen_target_pos).length();

	if (dist_to_last_seen < DASH_RADIUS) {
		ai_state.has_dashed_for_last_seen = true;
		return true;
	}

	return false;
}
