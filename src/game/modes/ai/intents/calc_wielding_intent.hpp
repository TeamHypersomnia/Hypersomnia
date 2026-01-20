#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/intents/calc_movement_flags.hpp"
#include "game/modes/ai/tasks/find_best_weapon.hpp"

inline bool should_holster_weapons(const ai_behavior_variant& behavior, const bool nav_near_end = false) {
	if (::is_camping_on_waypoint(behavior)) {
		return false;
	}

	/*
		Defusing requires bare hands - holster weapons when is_defusing.
	*/
	if (const auto* defuse = ::get_behavior_if<ai_behavior_defuse>(behavior)) {
		if (defuse->is_defusing) {
			return true;
		}
		else {
			return nav_near_end;
		}
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

/*
	Result of weapon wielding calculation.
*/

struct wielding_intent_result {
	wielding_setup desired_wielding;
	bool should_change = false;
};

/*
	Stateless calculation of the desired weapon wielding.
	
	This function determines what weapon the bot should wield based on:
	- Current behavior type (should holster or not)
	- Current wielding state
	- Available weapons in inventory
	
	Returns the desired wielding setup if it differs from current.
*/

template <typename CharacterHandle>
inline wielding_intent_result calc_wielding_intent(
	const ai_behavior_variant& behavior,
	CharacterHandle character_handle,
	const bool nav_near_end = false
) {
	wielding_intent_result result;
	const auto& cosm = character_handle.get_cosmos();

	const bool should_holster = ::should_holster_weapons(behavior, nav_near_end);
	const auto current_wielding = wielding_setup::from_current(character_handle);
	const bool has_bare_hands = current_wielding.is_bare_hands(cosm);

	if (should_holster && !has_bare_hands) {
		result.desired_wielding = wielding_setup::bare_hands();
		result.should_change = true;
	}
	else if (!should_holster && has_bare_hands) {
		const auto best_weapon = ::find_best_weapon(character_handle);

		if (best_weapon.is_set()) {
			result.desired_wielding = wielding_setup::bare_hands();
			result.desired_wielding.hand_selections[0] = best_weapon;
			result.should_change = true;
		}
	}

	return result;
}
