#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"

template <class E>
wielding_result inventory_mixin<E>::swap_wielded_items() const {
	const auto& self = *static_cast<const E*>(this);

	wielding_result result;

	const bool both_hands_available = self.get_hand_no(0).alive() && self.get_hand_no(1).alive();

	if (both_hands_available) {
		const auto in_primary = self.get_if_any_item_in_hand_no(0);
		const auto in_secondary = self.get_if_any_item_in_hand_no(1);

		auto& transfers = result.transfers;

		if (in_primary.alive() && in_secondary.alive()) {
			transfers = swap_slots_for_items(in_primary, in_secondary);
		}
		else if (in_primary.alive()) {
			transfers.push_back(item_slot_transfer_request::standard(in_primary, self.get_secondary_hand()));
		}
		else if (in_secondary.alive()) {
			transfers.push_back(item_slot_transfer_request::standard(in_secondary, self.get_primary_hand()));
		}

		result.result = wielding_result::type::SUCCESSFUL;
	}

	result.play_effects_only_in_first();

	for (auto& r : result.transfers) {
		r.params.play_transfer_particles = false;
	}

	return result;
}

template <class E>
wielding_result inventory_mixin<E>::make_wielding_transfers_for(hand_selections_array selections) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	for (entity_id& ss : selections) {
		if (!cosm[ss]) {
			ss = {};
		}
	}

	const auto first_held = self.get_hand_no(0).get_item_if_any().get_id();
	const auto second_held = self.get_hand_no(1).get_item_if_any().get_id();

	if (first_held == selections[1] && second_held == selections[0]) {
		/* Required items are swapped. */
		return swap_wielded_items();
	}

	wielding_result result;
	result.result = wielding_result::type::THE_SAME_SETUP;

	if (auto move_to_secondary_and_draw = 
		is_akimbo(cosm, selections)
		&& first_held == selections[1]
		&& second_held == entity_id()
	) {
		result.transfers.push_back(item_slot_transfer_request::standard(
			self.get_hand_no(0).get_item_if_any(), 
			self.get_hand_no(1)
		));

		result.transfers.push_back(item_slot_transfer_request::standard(
			selections[0],
			self.get_hand_no(0)
		));

		result.result = wielding_result::type::SUCCESSFUL;
		result.play_effects_only_in_last();
		return result;
	}

	entity_id try_to_holster_after_draw;
	augs::constant_size_vector<item_slot_transfer_request, hand_count_v> holsters;
	augs::constant_size_vector<item_slot_transfer_request, hand_count_v> draws;

	std::size_t si = 0;
	for (std::size_t i = 0; i < selections.size(); ++i) {
		const auto hand = self.get_hand_no(i);

		const auto item_for_hand = cosm[selections[si]];
		const auto item_in_hand = hand.get_item_if_any();

		const bool identical_outcome =
			(item_in_hand.dead() && item_for_hand.dead())
			|| item_in_hand == item_for_hand
		;

		if (identical_outcome) {
			++si;
			continue;
		}

		auto insert_for_hand = [&]() {
			if (item_for_hand.alive()) {
				draws.push_back(item_slot_transfer_request::standard(item_for_hand, hand));
			}

			++si;
		};

		if (item_in_hand.alive()) {
			const auto holstering_slot = self.find_holstering_slot_for(item_in_hand);

			if (holstering_slot.alive()) {
				holsters.push_back(item_slot_transfer_request::standard(item_in_hand, holstering_slot));
				insert_for_hand();
			}
			else {
				try_to_holster_after_draw = item_in_hand;
			}
		}
		else {
			insert_for_hand();
		}

		result.result = wielding_result::type::SUCCESSFUL;
	}

	concatenate(result.transfers, holsters);
	concatenate(result.transfers, draws);

	result.play_effects_only_in_last();

	if (try_to_holster_after_draw.is_set()) {
		if (draws.size() == 1) {
			const auto slot_drawn_from = cosm[draws[0].item].get_current_slot();

			result.transfers.push_back(item_slot_transfer_request::standard(
				try_to_holster_after_draw, 
				slot_drawn_from
			));
			
			auto& r = result.transfers.back();

			r.params.play_transfer_sounds = false;
			r.params.play_transfer_particles = false;
		}
	}

	return result;
}

