#pragma once
#include "game/detail/use_interaction_logic.h"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/detail/inventory/firearm_limit.h"
#include "game/detail/inventory/drop_oldest_items_to_fit.hpp"
#include "game/detail/inventory/perform_wielding.hpp"
#include "game/detail/inventory/wielding_setup.hpp"

inline interaction_result_type item_pickup::process(const logic_step step, const entity_id character) const {
	auto& cosm = step.get_cosmos();

	/*
		Pickup delays are a UX matter and should not slow down with logic speed.
	*/

	const auto clk = cosm.get_clock().get_real_clock();
	const auto picker = cosm[character];

	auto handle_pickup = [&](const auto& typed_item) {
		picker.dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_picker) {
			auto& transfers = typed_picker.template get<components::item_slot_transfers>();
			const auto& pick_list = transfers.only_pick_these_items;
			const bool found_on_subscription_list = found_in(pick_list, typed_item.get_id());

			if (/* item_subscribed */
				(pick_list.empty() && transfers.pick_all_touched_items_if_list_to_pick_empty)
				|| found_on_subscription_list
			) {
				const bool can_pick_already = transfers.pickup_timeout.try_to_fire_and_reset(clk);

				if (!can_pick_already) {
					return;
				}

				const auto picked_id = entity_id(typed_item.get_id());

				/*
					A firearm picked up while the hands hold no firearm goes into hands,
					hiding whatever else they hold (a grenade, a knife).
				*/

				auto wield_picked_firearm_if_hands_hold_none = [&]() {
					if (!::is_firearm_like(typed_item) || typed_item.get_owning_transfer_capability() != typed_picker) {
						return;
					}

					if (typed_item.get_current_slot().is_hand_slot()) {
						return;
					}

					if (::find_wielded_firearm_to_swap(typed_picker).is_set()) {
						return;
					}

					auto requested_wield = wielding_setup::bare_hands();
					requested_wield.hand_selections[0] = picked_id;

					::perform_wielding(step, typed_picker, requested_wield);
				};

				if (const auto swapped_firearm = cosm[::find_firearm_to_swap_for(typed_picker, typed_item)]) {
					/*
						Over the firearm limit: drop the swapped firearm and take the picked one in its place -
						into the same hand, or onto the back.
					*/

					auto requested_wield = wielding_setup::from_current(typed_picker);

					const bool swapped_from_hands = 
						requested_wield.hand_selections[0] == swapped_firearm.get_id()
						|| requested_wield.hand_selections[1] == swapped_firearm.get_id()
					;

					for (auto& h : requested_wield.hand_selections) {
						if (h == swapped_firearm.get_id()) {
							h = picked_id;
						}
					}

					if (!perform_transfer(item_slot_transfer_request::drop(swapped_firearm), step).result.is_successful()) {
						return;
					}

					const auto pickup_slot = typed_picker.find_pickup_target_slot_for(typed_item, { slot_finding_opt::OMIT_MOUNTED_SLOTS });

					if (pickup_slot.alive()) {
						perform_transfer(item_slot_transfer_request::standard(picked_id, pickup_slot), step);

						if (swapped_from_hands) {
							::perform_wielding(step, typed_picker, requested_wield);
						}
						else {
							wield_picked_firearm_if_hands_hold_none();
						}
					}

					return;
				}

				auto find_pickup_slot = [&]() {
					return typed_picker.find_pickup_target_slot_for(typed_item, { slot_finding_opt::OMIT_MOUNTED_SLOTS }).get_id();
				};

				const auto pickup_slot_id = [&]() {
					const auto found = find_pickup_slot();

					if (!found.is_set() && !::is_firearm_like(typed_item)) {
						if (::drop_oldest_items_to_fit(step, typed_picker, typed_item)) {
							return find_pickup_slot();
						}
					}

					return found;
				}();

				const auto pickup_slot = cosm[pickup_slot_id];

				if (pickup_slot.alive()) {
					perform_transfer(item_slot_transfer_request::standard(picked_id, pickup_slot), step);
					wield_picked_firearm_if_hands_hold_none();
				}
			}
		});
	};

	cosm[item].dispatch_on_having_all<invariants::item>(handle_pickup);

	return interaction_result_type::CAN_BEGIN;
}
