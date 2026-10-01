#pragma once
#include "game/components/item_component.h"
#include "game/detail/weapon_like.h"
#include "game/detail/calc_ammo_info.hpp"

enum class magazine_ammo_state {
	NOT_A_MAGAZINE,
	NORMAL,
	LOW,
	EMPTY
};

/*
	The rule that drops magazines with fewer bullets than the gun's low ammo cue threshold to the ground on reload.
*/

inline bool is_below_low_ammo_threshold(const int charges, const unsigned threshold) {
	return charges < static_cast<int>(threshold);
}

/*
	LOW when there are fewer bullets than the gun's low ammo cue threshold -
	see invariants::item::low_ammo_threshold. EMPTY when there are none.
*/

template <class E>
magazine_ammo_state calc_magazine_ammo_state(const E& typed_item) {
	if (!::is_magazine_like(typed_item)) {
		return magazine_ammo_state::NOT_A_MAGAZINE;
	}

	const auto charges = ::count_charges_in_deposit(typed_item);

	if (charges == 0) {
		return magazine_ammo_state::EMPTY;
	}

	if (const auto item_def = typed_item.template find<invariants::item>()) {
		if (::is_below_low_ammo_threshold(charges, item_def->low_ammo_threshold)) {
			return magazine_ammo_state::LOW;
		}
	}

	return magazine_ammo_state::NORMAL;
}

template <class E>
bool is_low_ammo_magazine(const E& typed_item) {
	const auto state = ::calc_magazine_ammo_state(typed_item);
	return state == magazine_ammo_state::LOW || state == magazine_ammo_state::EMPTY;
}
