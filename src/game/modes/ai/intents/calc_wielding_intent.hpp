#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/intents/should_helpers.hpp"
#include "game/modes/ai/tasks/find_best_weapon.hpp"

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
	CharacterHandle character_handle
) {
	wielding_intent_result result;
	const auto& cosm = character_handle.get_cosmos();

	const bool should_holster = ::should_holster_weapons(behavior);
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
