#pragma once
#include <variant>

#include "game/modes/ai/behaviors/ai_behavior_idle.hpp"
#include "game/modes/ai/behaviors/ai_behavior_combat.hpp"
#include "game/modes/ai/behaviors/ai_behavior_patrol.hpp"
#include "game/modes/ai/behaviors/ai_behavior_defuse.hpp"
#include "game/modes/ai/behaviors/ai_behavior_retrieve_bomb.hpp"
#include "game/modes/ai/behaviors/ai_behavior_plant.hpp"

/*
	Variant type holding all possible AI behaviors.
	Used as the `last_behavior` in arena_mode_ai_state.
	
	The default behavior is ai_behavior_idle.
	
	Note: ai_behavior_push has been merged into ai_behavior_patrol.
	Patrol now has an optional push_waypoint that takes priority.
*/

using ai_behavior_variant = std::variant<
	ai_behavior_idle,
	ai_behavior_combat,
	ai_behavior_patrol,
	ai_behavior_defuse,
	ai_behavior_retrieve_bomb,
	ai_behavior_plant
>;

/*
	Helper to check if a variant holds a specific behavior type.
*/

template <typename T>
inline bool is_behavior(const ai_behavior_variant& behavior) {
	return std::holds_alternative<T>(behavior);
}

/*
	Helper to get behavior type if it matches, returns nullptr otherwise.
*/

template <typename T>
inline T* get_behavior_if(ai_behavior_variant& behavior) {
	return std::get_if<T>(&behavior);
}

template <typename T>
inline const T* get_behavior_if(const ai_behavior_variant& behavior) {
	return std::get_if<T>(&behavior);
}
