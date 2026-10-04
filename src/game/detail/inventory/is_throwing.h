#pragma once
#include "game/components/item_slot_transfers_component.h"
#include "game/detail/explosive/like_explosive.h"

/*
	Whether a character is about to throw, or is throwing,
	an armed explosive or the melee weapons it holds.
*/

template <class E>
bool is_throwing_explosive(const E& character) {
	const auto& cosm = character.get_cosmos();

	for (const auto& w : character.get_wielded_items()) {
		if (::is_armed_explosive(cosm[w])) {
			return true;
		}
	}

	return false;
}

template <class E>
bool is_throwing_melee(const E& character) {
	if (const auto transfers = character.template find<components::item_slot_transfers>()) {
		return transfers->when_throw_requested.was_set();
	}

	return false;
}
