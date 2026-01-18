#pragma once
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/behaviors/ai_target_tracking.hpp"

/*
	Stateless intent calculations that std::visit the behavior variant.
	These functions centralize the logic for determining bot intents.

	Note: ai_behavior_push has been merged into ai_behavior_patrol.
	The patrol's push_waypoint field now indicates push state.
*/

inline bool should_sprint(const ai_behavior_variant& behavior, const bool nav_can_sprint) {
	return std::visit([nav_can_sprint](const auto& b) -> bool {
		using T = std::decay_t<decltype(b)>;

		if constexpr (std::is_same_v<T, ai_behavior_combat>) {
			return true;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_patrol>) {
			return nav_can_sprint && b.is_going_far();
		}
		else if constexpr (std::is_same_v<T, ai_behavior_retrieve_bomb>) {
			return true;
		}
		else {
			return false;
		}
	}, behavior);
}

inline bool is_camping_on_waypoint(const ai_behavior_variant& behavior) {
	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		return patrol->is_camping();
	}

	return false;
}

inline bool should_walk_silently(const ai_behavior_variant& behavior) {
	if (::is_camping_on_waypoint(behavior)) {
		return true;
	}

	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		/* Always walk silently when camping (twitching). */
		if (patrol->in_motion() && !patrol->is_going_far()) {
			return patrol->walk_silently_to_next_waypoint;
		}
	}

	return false;
}

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

inline bool should_holster_weapons(const ai_behavior_variant& behavior) {
	if (::is_camping_on_waypoint(behavior)) {
		return false;
	}

	return std::visit([&](const auto& b) -> bool {
		using T = std::decay_t<decltype(b)>;

		if constexpr (std::is_same_v<T, ai_behavior_combat>) {
			return false;
		}
		else {
			return ::should_sprint(behavior, true);
		}
	}, behavior);
}

