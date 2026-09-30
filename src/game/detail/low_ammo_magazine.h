#pragma once
#include "augs/templates/container_templates.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/gun_component.h"
#include "game/components/container_component.h"
#include "game/detail/weapon_like.h"
#include "game/detail/calc_ammo_info.hpp"

enum class magazine_ammo_state {
	NOT_A_MAGAZINE,
	NORMAL,
	LOW,
	EMPTY
};

/*
	A magazine lying on the ground doesn't know its gun,
	so find the gun's low ammo cue threshold by the magazine's flavour.
	0 if no gun takes this magazine.
*/

inline unsigned calc_low_ammo_threshold_of_magazine(const cosmos& cosm, const entity_flavour_id magazine_flavour) {
	auto result = 0u;

	cosm.for_each_flavour_having<invariants::gun, invariants::container>(
		[&](const auto&, const auto& gun_flavour) {
			const auto& slots = gun_flavour.template get<invariants::container>().slots;

			if (const auto mag_slot = mapped_or_nullptr(slots, slot_function::GUN_DETACHABLE_MAGAZINE)) {
				if (mag_slot->only_allow_flavour.is_set() && entity_flavour_id(mag_slot->only_allow_flavour) == magazine_flavour) {
					result = gun_flavour.template get<invariants::gun>().num_last_bullets_to_trigger_low_ammo_cue;
				}
			}
		}
	);

	return result;
}

/*
	LOW when there are fewer bullets than the gun's low ammo cue threshold -
	the same rule that drops such magazines to the ground on reload.
	EMPTY when there are none.
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

	const auto threshold = ::calc_low_ammo_threshold_of_magazine(typed_item.get_cosmos(), entity_flavour_id(typed_item.get_flavour_id()));

	if (charges < static_cast<int>(threshold)) {
		return magazine_ammo_state::LOW;
	}

	return magazine_ammo_state::NORMAL;
}

template <class E>
bool is_low_ammo_magazine(const E& typed_item) {
	const auto state = ::calc_magazine_ammo_state(typed_item);
	return state == magazine_ammo_state::LOW || state == magazine_ammo_state::EMPTY;
}
