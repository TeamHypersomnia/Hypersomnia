#pragma once
#include "game/components/gun_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/melee_component.h"
#include "game/detail/explosive/like_explosive.h"
#include "view/rendering_scripts/throw_trajectory_flags.h"

/*
	Right before a throw, two items might be wielded at once,
	e.g. a gun and a grenade drawn with a throw key.
	Only the item about to be thrown is aimed with then -
	its laser or its trajectory is shown, and not those of the other item.

	Unless aim_side_throws is false and the other item is a gun:
	then the gun keeps being aimed with, so that its laser does not flicker
	for the short while the item is thrown from the other hand.
*/

template <class H, class I>
bool is_aimed_with(const H& character, const I& item, const bool aim_side_throws) {
	const auto& cosm = character.get_cosmos();
	const auto wielded_items = character.get_wielded_items();

	const bool throwing_explosive = [&]() {
		for (const auto& w : wielded_items) {
			if (::is_armed_explosive(cosm[w])) {
				return true;
			}
		}

		return false;
	}();

	const bool throwing_melee = [&]() {
		if (const auto transfers = character.template find<components::item_slot_transfers>()) {
			return transfers->when_throw_requested.was_set();
		}

		return false;
	}();

	if (!aim_side_throws && (throwing_explosive || throwing_melee)) {
		const bool gun_wielded = [&]() {
			for (const auto& w : wielded_items) {
				if (const auto wielded = cosm[w]; wielded.alive() && wielded.template has<components::gun>()) {
					return true;
				}
			}

			return false;
		}();

		if (gun_wielded) {
			return item.template has<components::gun>();
		}
	}

	if (throwing_explosive) {
		return ::is_armed_explosive(item);
	}

	if (throwing_melee) {
		return item.template has<components::melee>();
	}

	return true;
}
