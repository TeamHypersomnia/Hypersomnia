#pragma once
#include "game/cosmos/entity_handle.h"
#include "game/components/item_component.h"
#include "game/components/gun_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/components/melee_component.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/detail/weapon_like.h"

/*
	Find the most expensive weapon in the bot's inventory.
	Returns the item id if found, otherwise returns dead entity_id.
*/

template <class E>
inline entity_id find_best_weapon(const E& character_handle) {
	entity_id best_weapon;
	money_type best_price = 0;

	character_handle.for_each_contained_item_recursive(
		[&](const auto& item) {
			if (!item.template has<components::gun>()) {
				return recursive_callback_result::CONTINUE_AND_RECURSE;
			}

			if (const auto item_def = item.template find<invariants::item>()) {
				const auto price = item_def->standard_price;

				if (price > best_price) {
					best_price = price;
					best_weapon = item.get_id();
				}
			}

			return recursive_callback_result::CONTINUE_DONT_RECURSE;
		}
	);

	return best_weapon;
}
