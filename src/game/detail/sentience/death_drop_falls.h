#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/item_component.h"
#include "game/detail/inventory/item_falling.h"

/*
	A character dying with a gun in hand drops it like a thrown away item - it hops off the floor,
	with the sounds of hitting it - except for the basic pistols everyone has
	(invariants::item::falls_when_dropped_on_death).
	Pass the items it held, after they were dropped.
*/

template <class C>
void start_dropped_on_death_falls(const logic_step step, const C& held_items) {
	auto& cosm = step.get_cosmos();

	for (const auto& id : held_items) {
		const auto item = cosm[id];

		if (item.dead()) {
			continue;
		}

		item.template dispatch_on_having_all<invariants::item>([&](const auto& typed_item) {
			if (!typed_item.template get<invariants::item>().falls_when_dropped_on_death) {
				return;
			}

			auto& fall = typed_item.template get<components::item>().get_fall();

			::start_falling_like_dropped(
				fall,
				DROPPED_ITEM_FALL_SECS,
				DROPPED_ITEM_FALL_HEIGHT,
				cosm.get_nontemporal_rng_seed_for(typed_item),
				cosm.get_timestamp()
			);
		});
	}
}
