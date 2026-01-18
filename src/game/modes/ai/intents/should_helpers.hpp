#pragma once
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/behaviors/ai_target_tracking.hpp"

/*
	Stateless intent calculations that std::visit the behavior variant.
	These functions centralize the logic for determining bot intents.
*/

/*
	Determines if the bot should holster weapons.
	Returns true when patrolling/traveling to first waypoint without combat.
	Always false in COMBAT.
*/

inline bool should_holster_weapons(const ai_behavior_variant& behavior) {
	return std::visit([](const auto& b) -> bool {
		using T = std::decay_t<decltype(b)>;

		if constexpr (std::is_same_v<T, ai_behavior_combat>) {
			return false;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_patrol>) {
			return b.going_to_first_waypoint;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_push>) {
			return true;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_plant>) {
			return true;
		}
		else {
			return false;
		}
	}, behavior);
}

/*
	Determines if the bot should sprint.
	- First/switching waypoint in patrol
	- Going to PUSH waypoint
	- In COMBAT (chasing)
	- Retrieving bomb
	
	Additionally, for patrol/push states, sprinting is only allowed when
	the movement direction is mostly parallel to the pathfinding direction
	(can_sprint from navigation result).
*/

inline bool should_sprint(const ai_behavior_variant& behavior, const bool nav_can_sprint = true) {
	return std::visit([nav_can_sprint](const auto& b) -> bool {
		using T = std::decay_t<decltype(b)>;

		if constexpr (std::is_same_v<T, ai_behavior_combat>) {
			return true;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_patrol>) {
			return b.going_to_first_waypoint && nav_can_sprint;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_push>) {
			return nav_can_sprint;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_retrieve_bomb>) {
			return true;
		}
		else {
			return false;
		}
	}, behavior);
}

/*
	Determines if the bot is currently camping on a waypoint.
*/

inline bool is_camping(const ai_behavior_variant& behavior) {
	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		return patrol->is_camping();
	}

	return false;
}

/*
	Determines if the bot should walk silently.
	Only when patrolling (and not going to first waypoint).
	85% chance to walk silently when choosing next waypoint.
	Always walk silently when camping.
*/

inline bool should_walk_silently(const ai_behavior_variant& behavior) {
	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		/* Always walk silently when camping (twitching). */
		if (patrol->is_camping()) {
			return true;
		}

		if (patrol->going_to_first_waypoint) {
			return false;
		}

		return patrol->walk_silently_to_next_waypoint;
	}

	return false;
}

/*
	Check if we should dash when crossing last_seen_target_pos.
	Only applies in COMBAT state.
*/

inline bool should_dash_for_combat(
	ai_behavior_variant& behavior,
	const ai_target_tracking& combat_target,
	const vec2 bot_pos
) {
	auto* combat = ::get_behavior_if<ai_behavior_combat>(behavior);

	if (combat == nullptr) {
		return false;
	}

	if (combat->has_dashed_for_position(combat_target.last_seen_pos)) {
		return false;
	}

	constexpr float DASH_RADIUS = 100.0f;
	const auto dist_to_last_seen = (bot_pos - combat_target.last_seen_pos).length();

	if (dist_to_last_seen < DASH_RADIUS) {
		combat->mark_dashed_for_position(combat_target.last_seen_pos);
		return true;
	}

	return false;
}
