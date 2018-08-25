#pragma once
#include "game/detail/damage_origin.h"

template <class E>
void damage_origin::copy_sender_from(const E& causing_handle) {
	if (const auto s = causing_handle.template find<components::sender>()) {
		sender = *s;
	}
	else {
		sender.set(causing_handle);
	}
}

template <class E>
const_entity_handle damage_origin::get_guilty_of_damaging(const E& victim_handle) const {
	const auto faction = victim_handle.get_official_faction();
	auto& cosm = victim_handle.get_cosmos();

	auto is_delayed_enough_explosive = [&](const auto flavour_id) {
		if (!flavour_id.is_set()) {
			return false;
		}

		return flavour_id.dispatch([&](const auto& typed_id) {
			if (const auto flavour = cosm.find_flavour(typed_id)) {
				if (const auto hand_fuse = flavour->template find<invariants::hand_fuse>()) {
					return hand_fuse->is_like_plantable_bomb();
				}
			}

			return false;
		});
	};

	if (sender.faction_of_sender == faction) {
		/* 
			Case: direct cause is an explosion body of a bomb planted by the same faction.
			The sender is the bomb and the sender faction is propagated from the sender of the bomb.
		*/

		if (is_delayed_enough_explosive(sender.direct_sender_flavour)) {
			return victim_handle;
		}

		/* 
			Case: direct cause is a bomb planted by the same faction.
			The sender is the player and its faction is set.
		*/

		if (is_delayed_enough_explosive(cause.flavour)) {
			return victim_handle;
		}
	}

	return cosm[sender.capability_of_sender];
}
