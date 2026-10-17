#pragma once
#include <cstdint>
#include <limits>
#include "game/cosmos/logic_step.h"
#include "game/enums/slot_function.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/detail/inventory/firearm_limit.h"
#include "game/detail/weapon_like.h"
#include "game/detail/flavour_scripts.h"
#include "game/detail/explosive/like_explosive.h"

/*
	Everything but firearms and wearables goes into the one personal deposit (a backpack only enlarges it).
	When something does not fit, items fall out of it - ammo for none of the carried firearms first,
	then the ones put there the longest ago.
*/

template <class E>
auto find_personal_deposit(const E& character) {
	if (const auto personal_wearable = character[slot_function::PERSONAL_DEPOSIT].get_item_if_any()) {
		return personal_wearable[slot_function::ITEM_DEPOSIT];
	}

	return character.get_cosmos()[inventory_slot_id()];
}

/*
	Ammo that fits none of the carried firearms is useless, so it falls out first.
*/

template <class E, class I>
bool is_useless_ammo(const E& character, const I& item) {
	if (!::is_ammo_like(item)) {
		return false;
	}

	bool fits_some_firearm = false;

	auto fits = [&](const auto& slot) {
		return slot.alive() && slot->is_category_compatible_with(item);
	};

	::for_each_carried_firearm(character, [&](const auto& firearm) {
		for (const auto type : { slot_function::GUN_DETACHABLE_MAGAZINE, slot_function::GUN_CHAMBER, slot_function::GUN_CHAMBER_MAGAZINE }) {
			const auto slot = firearm[type];

			if (slot.dead()) {
				continue;
			}

			if (fits(slot)) {
				fits_some_firearm = true;
			}

			if (const auto mag = slot.get_item_if_any()) {
				if (fits(mag[slot_function::ITEM_DEPOSIT])) {
					fits_some_firearm = true;
				}
			}
		}
	});

	return !fits_some_firearm;
}

/*
	Space taken in the personal deposit by ammo for none of the carried firearms -
	as good as free, since it falls out first.
*/

template <class E>
inventory_space_type calc_deposit_space_of_useless_ammo(const E& character) {
	const auto& cosm = character.get_cosmos();
	const auto deposit = ::find_personal_deposit(character);

	if (deposit.dead()) {
		return 0;
	}

	auto total = inventory_space_type(0);

	for (const auto& candidate_id : deposit.get_items_inside()) {
		const auto candidate = cosm[candidate_id];

		if (::is_useless_ammo(character, candidate)) {
			total += ::calc_space_occupied_with_children(candidate);
		}
	}

	return total;
}

template <class E>
entity_id find_next_to_fall_out(const E& character) {
	const auto& cosm = character.get_cosmos();
	const auto deposit = ::find_personal_deposit(character);

	if (deposit.dead()) {
		return entity_id();
	}

	entity_id next;
	auto next_step = std::numeric_limits<uint32_t>::max();
	bool next_useless = false;

	for (const auto& candidate_id : deposit.get_items_inside()) {
		const auto candidate = cosm[candidate_id];
		const auto stowed_step = candidate.when_last_transferred().step;
		const bool useless = ::is_useless_ammo(character, candidate);

		/*
			Ties broken by id - the order of items inside is inferred, so it may differ after reinference.
		*/

		const bool falls_out_sooner =
			!next.is_set()
			|| (useless && !next_useless)
			|| (useless == next_useless && stowed_step < next_step)
			|| (useless == next_useless && stowed_step == next_step && candidate_id < next)
		;

		if (falls_out_sooner) {
			next = candidate_id;
			next_step = stowed_step;
			next_useless = useless;
		}
	}

	return next;
}

/*
	Drops items out of the personal deposit until the condition holds.
	Returns whether it holds.
*/

template <class E, class F>
bool drop_from_deposit_until(const logic_step step, const E& character, F&& condition) {
	while (!condition()) {
		const auto next = ::find_next_to_fall_out(character);

		if (!next.is_set()) {
			return false;
		}

		if (!perform_transfer(item_slot_transfer_request::drop(next), step).result.is_successful()) {
			return false;
		}
	}

	return true;
}

template <class E, class I>
bool can_make_room_for(const E& character, const I& item) {
	if (::is_firearm_like(item)) {
		/*
			A firearm never goes into the deposit (see query_containment_result) -
			emptying it would drop everything for nothing.
		*/

		return false;
	}

	const auto deposit = ::find_personal_deposit(character);

	return
		deposit.alive()
		&& deposit->is_category_compatible_with(item)
		&& deposit.calc_total_space() >= ::calc_space_occupied_with_children(item)
	;
}

/*
	Returns whether the item fits now.
*/

template <class E, class I>
bool drop_oldest_items_to_fit(const logic_step step, const E& character, const I& item) {
	const auto stowing_types = candidate_holster_types {
		candidate_holster_type::WEARABLES,
		candidate_holster_type::CONTAINERS
	};

	/*
		A whole stack has to fit - otherwise a part of it would be left behind.
	*/

	auto fits = [&]() {
		return character.find_slot_for(item, stowing_types, { slot_finding_opt::ALL_CHARGES_MUST_FIT, slot_finding_opt::OMIT_MOUNTED_SLOTS }).alive();
	};

	if (fits()) {
		return true;
	}

	if (!::can_make_room_for(character, item)) {
		return false;
	}

	return ::drop_from_deposit_until(step, character, fits);
}

/*
	Taking off the backpack shrinks the personal deposit - whatever no longer fits falls out.
*/

template <class E>
void drop_items_over_deposit_capacity(const logic_step step, const E& character) {
	const auto deposit = ::find_personal_deposit(character);

	if (deposit.dead()) {
		return;
	}

	::drop_from_deposit_until(step, character, [&]() {
		return deposit.calc_space_occupied_by_children() <= deposit.calc_total_space();
	});
}

/*
	Deposit space that the items held in hands would take when hidden.
	Counted the same for showing what fits in the shop and for making room after a purchase,
	so that what the shop shows as fitting never makes anything fall out later.
	Conservative: an item that might go to a wearable slot instead (a knife) still counts.
*/

template <class E>
inventory_space_type calc_deposit_space_needed_for_hands(const E& character) {
	const auto& cosm = character.get_cosmos();
	const auto deposit = ::find_personal_deposit(character);

	if (deposit.dead()) {
		return 0;
	}

	auto needed = inventory_space_type(0);

	for (const auto& held : character.get_wielded_items()) {
		const auto held_item = cosm[held];

		/*
			Ammo is held only while reloading - it goes into the firearm, not the deposit.
		*/

		if (held_item.dead() || ::is_firearm_like(held_item) || ::is_like_plantable_bomb(held_item) || ::is_ammo_like(held_item)) {
			continue;
		}

		if (deposit->is_category_compatible_with(held_item)) {
			needed += ::calc_space_occupied_with_children(held_item);
		}
	}

	return needed;
}

/*
	After a purchase, the deposit keeps room for everything held in hands,
	so that hiding it later never drops anything.
*/

template <class E>
void reserve_deposit_space_for_hands(const logic_step step, const E& character) {
	const auto deposit = ::find_personal_deposit(character);

	if (deposit.dead()) {
		return;
	}

	const auto needed = ::calc_deposit_space_needed_for_hands(character);

	if (needed > deposit.calc_total_space()) {
		return;
	}

	::drop_from_deposit_until(step, character, [&]() {
		return deposit.calc_local_space_available() >= needed;
	});
}

/*
	A purchase that does not fit is still made when the deposit could take it after items fall out.
*/

template <class E>
bool can_make_room_for_purchase(const E& character, const item_flavour_id& flavour_id) {
	const auto& cosm = character.get_cosmos();
	const auto deposit = ::find_personal_deposit(character);

	if (deposit.dead()) {
		return false;
	}

	const auto needed_for_hands = ::calc_deposit_space_needed_for_hands(character);

	return cosm.on_flavour(flavour_id, [&](const auto& typed_flavour) {
		const auto& categories = typed_flavour.template get<invariants::item>().categories_for_slot_compatibility;

		/*
			Room for what the hands hold too - see reserve_deposit_space_for_hands,
			otherwise the purchase itself would fall out right after.
		*/

		return
			deposit->is_category_compatible_with(flavour_id, categories)
			&& deposit.calc_total_space() >= ::calc_space_occupied_of_purchased(cosm, flavour_id) + needed_for_hands
		;
	});
}
