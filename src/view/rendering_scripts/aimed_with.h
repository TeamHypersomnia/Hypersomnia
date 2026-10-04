#pragma once
#include "game/components/gun_component.h"
#include "game/components/melee_component.h"
#include "game/detail/explosive/like_explosive.h"
#include "game/detail/inventory/is_throwing.h"
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

	const bool throwing_explosive = ::is_throwing_explosive(character);
	const bool throwing_melee = ::is_throwing_melee(character);

	if (!aim_side_throws && (throwing_explosive || throwing_melee)) {
		const bool gun_wielded = [&]() {
			for (const auto& w : character.get_wielded_items()) {
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

/*
	Whether the trajectory of a held item is shown - see throw_trajectory_flags:
	with only_when_throwing, only that of an armed grenade or of a melee weapon about to be thrown.
*/

template <class H, class I>
bool is_throw_trajectory_shown(const H& character, const I& item, const throw_trajectory_flags& flags) {
	if (!flags.shown_for(item) || !::is_aimed_with(character, item, flags.during_side_throws)) {
		return false;
	}

	if (flags.only_when_throwing) {
		if (::is_armed_explosive(item)) {
			return true;
		}

		return item.template has<components::melee>() && ::is_throwing_melee(character);
	}

	return true;
}
