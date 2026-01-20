#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/intents/calc_movement_flags.hpp"
#include "game/modes/ai/tasks/find_best_weapon.hpp"
#include "game/components/hand_fuse_component.h"
#include "game/detail/explosive/like_explosive.h"

/*
	Find the bomb entity in the bot's inventory (if any).
	Returns the bomb item id if found, otherwise returns dead entity_id.
*/

template <class E>
inline entity_id find_bomb_in_inventory(const E& character_handle) {
	entity_id bomb_id;

	character_handle.for_each_contained_item_recursive(
		[&](const auto& item) {
			if (bomb_id.is_set()) {
				return;
			}

			if (::is_like_plantable_bomb(item)) {
				bomb_id = item.get_id();
			}
		}
	);

	return bomb_id;
}

inline bool should_holster_weapons(const ai_behavior_variant& behavior, const bool nav_nearing_end) {
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
			return nav_nearing_end;
		}
	}

	/*
		Planting requires holding the bomb - should NOT holster.
		Return false so the bomb wielding logic can pull out the bomb.
	*/
	if (const auto* plant = ::get_behavior_if<ai_behavior_plant>(behavior)) {
		if (plant->is_planting) {
			return false;
		}
		else {
			/*
				When nearing the plant location, also don't holster
				so we can pull out the bomb.
			*/
			return !nav_nearing_end;
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
	- Plant behavior: pull out bomb if nav_nearing_end or is_planting
	
	Returns the desired wielding setup if it differs from current.
*/

template <typename CharacterHandle>
inline wielding_intent_result calc_wielding_intent(
	const ai_behavior_variant& behavior,
	CharacterHandle character_handle,
	const bool nav_nearing_end
) {
	wielding_intent_result result;
	const auto& cosm = character_handle.get_cosmos();

	/*
		Special handling for plant behavior: pull out the bomb.
	*/
	if (const auto* plant = ::get_behavior_if<ai_behavior_plant>(behavior)) {
		const bool should_wield_bomb = plant->is_planting || nav_nearing_end;

		if (should_wield_bomb) {
			const auto bomb_id = ::find_bomb_in_inventory(character_handle);

			if (bomb_id.is_set()) {
				const auto current_wielding = wielding_setup::from_current(character_handle);

				/*
					Check if bomb is already wielded.
				*/
				const bool already_wielding_bomb = 
					current_wielding.hand_selections[0] == bomb_id ||
					current_wielding.hand_selections[1] == bomb_id
				;

				if (!already_wielding_bomb) {
					result.desired_wielding = wielding_setup::bare_hands();
					result.desired_wielding.hand_selections[0] = bomb_id;
					result.should_change = true;
					return result;
				}
			}
		}
		else {
			/*
				Not nearing plant location and not planting - wield best weapon.
			*/
			const auto current_wielding = wielding_setup::from_current(character_handle);
			const bool has_bare_hands = current_wielding.is_bare_hands(cosm);
			const auto best_weapon = ::find_best_weapon(character_handle);

			if (has_bare_hands && best_weapon.is_set()) {
				result.desired_wielding = wielding_setup::bare_hands();
				result.desired_wielding.hand_selections[0] = best_weapon;
				result.should_change = true;
			}

			return result;
		}

		return result;
	}

	const bool should_holster = ::should_holster_weapons(behavior, nav_nearing_end);
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
