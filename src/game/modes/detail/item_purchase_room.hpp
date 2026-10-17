#pragma once
#include "game/detail/inventory/firearm_limit.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/inventory/drop_oldest_items_to_fit.hpp"
#include "game/modes/detail/item_purchase_logic.hpp"

/*
	What a purchase takes and makes room for - only for the shop and the logic that buys,
	the rest of item_purchase_logic.hpp is included much wider.
*/

template <class E>
int num_carryable_pieces(
	const E& in_subject,
	const candidate_holster_types& opts,
	const item_flavour_id& item
) {
	int total_fitting = 0;

	const auto& cosm = in_subject.get_cosmos();

	cosm.on_flavour(
		item,
		[&](const auto& typed_flavour) {
			const auto& item_def = typed_flavour.template get<invariants::item>();

			if (::is_firearm_like(item_def.categories_for_slot_compatibility)) {
				/*
					Always buyable - buying over the limit drops the cheapest firearm we carry.
				*/

				total_fitting = 1;
				return;
			}

			const auto piece_occupied_space = calc_space_occupied_of_purchased(cosm, item);

			/*
				Hands never count as space for a purchase that is not a firearm,
				and the personal deposit keeps room for what the hands hold - see reserve_deposit_space_for_hands.
				Ammo for none of the carried firearms counts as free space - it is the first to fall out,
				so the shop shows a purchase as not fitting only when something useful would fall out.
			*/

			const auto deposit_id = ::find_personal_deposit(in_subject).get_id();
			const auto needed_for_hands = ::calc_deposit_space_needed_for_hands(in_subject);
			const auto freeable = ::calc_deposit_space_of_useless_ammo(in_subject);

			auto check_slot = [&](const auto& slot) {
				if (slot.dead() || slot.is_hand_slot()) {
					return;
				}

				if (slot->is_category_compatible_with(item, item_def.categories_for_slot_compatibility)) {
					if (slot->always_allow_exactly_one_item) {
						if (slot.is_empty_slot()) {
							++total_fitting;
						}
					}
					else {
						const bool is_deposit = slot.get_id() == deposit_id;
						const auto available = slot.calc_real_space_available() + (is_deposit ? freeable : inventory_space_type(0));
						const auto reserved = is_deposit ? needed_for_hands : inventory_space_type(0);

						if (available > reserved) {
							total_fitting += (available - reserved) / piece_occupied_space;
						}
					}
				}
			};

			in_subject.for_each_candidate_slot(
				opts,
				check_slot
			);
		}
	);

	return total_fitting;
}

/*
	Buying a firearm over the limit drops the cheapest firearm we carry,
	so that e.g. a second pistol never has to be thrown away by hand before buying a rifle.
*/

template <class E>
void drop_cheapest_firearms_to_fit_one_more(const logic_step step, const E& character) {
	while (::count_carried_firearms(character) >= ::calc_max_firearms(character)) {
		entity_id cheapest;
		money_type cheapest_price = 0;

		::for_each_carried_firearm(character, [&](const auto& firearm) {
			const auto price = firearm.find_price().value_or(0);

			if (!cheapest.is_set() || price < cheapest_price) {
				cheapest = firearm.get_id();
				cheapest_price = price;
			}
		});

		if (!cheapest.is_set()) {
			break;
		}

		if (!perform_transfer(item_slot_transfer_request::drop(cheapest), step).result.is_successful()) {
			break;
		}
	}
}
