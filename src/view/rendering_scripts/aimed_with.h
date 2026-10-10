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

	The thrown item is aimed with even when thrown from the hand other than a gun's - see is_side_throw.
*/

template <class H, class I>
bool is_aimed_with(const H& character, const I& item) {
	const bool throwing_explosive = ::is_throwing_explosive(character);
	const bool throwing_melee = ::is_throwing_melee(character);

	if (throwing_explosive) {
		return ::is_armed_explosive(item);
	}

	if (throwing_melee) {
		return item.template has<components::melee>();
	}

	return true;
}

/*
	Whether an item is about to be thrown from the hand other than a gun's.
*/

template <class H>
bool is_side_throw(const H& character) {
	if (!::is_throwing_explosive(character) && !::is_throwing_melee(character)) {
		return false;
	}

	const auto& cosm = character.get_cosmos();

	for (const auto& w : character.get_wielded_items()) {
		if (const auto wielded = cosm[w]; wielded.alive() && wielded.template has<components::gun>()) {
			return true;
		}
	}

	return false;
}

/*
	Whether the trajectory of a held item is drawn as a laser - see throw_trajectory_mode.
	A side throw is always drawn as a laser unless throw_trajectory_flags::during_side_throws -
	the gun's laser is only replaced by it for the short while.
*/

template <class H, class I>
bool is_throw_trajectory_laser(const H& character, const I& item, const throw_trajectory_flags& flags) {
	return flags.as_laser_for(item) || (!flags.during_side_throws && ::is_side_throw(character));
}

/*
	Whether the trajectory of a held item is shown - see throw_trajectory_flags:
	with only_when_throwing, only that of an armed grenade or of a melee weapon about to be thrown.
*/

template <class H, class I>
bool is_throw_trajectory_shown(const H& character, const I& item, const throw_trajectory_flags& flags) {
	if (!flags.shown_for(item) || !::is_aimed_with(character, item)) {
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
