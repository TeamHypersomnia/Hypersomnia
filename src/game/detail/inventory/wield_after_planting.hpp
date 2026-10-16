#pragma once
#include "game/components/item_slot_transfers_component.h"
#include "game/detail/inventory/perform_wielding.hpp"
#include "game/detail/inventory/wielding_setup.hpp"

/*
	Once the bomb leaves the hands planted:
	- a weapon left in the secondary hand goes back to the primary one,
	- with the hands left empty, what was wielded before the bomb is wielded back - like with LAST_USED_WEAPON.
*/

template <class E>
void wield_after_planting(const logic_step step, const E& character) {
	const auto transfers = character.template find<components::item_slot_transfers>();

	if (transfers == nullptr) {
		return;
	}

	const auto& cosm = character.get_cosmos();

	const wielding_setup previous = transfers->wield_after_planting;
	transfers->wield_after_planting = {};

	const auto current = wielding_setup::from_current(character);

	if (current.is_bare_hands(cosm)) {
		const auto viable_previous = previous.make_viable_setup(character);

		if (!viable_previous.is_bare_hands(cosm)) {
			::perform_wielding(step, character, viable_previous);
		}

		return;
	}

	if (cosm[current.hand_selections[0]].dead()) {
		::perform_wielding(step, character, current.get_switched_hands());
	}
}
