#pragma once
#include "augs/templates/logically_empty.h"

#define LOG_WIELDING 0

template <class... Args>
void WLD_LOG(Args&&... args) {
#if LOG_WIELDING
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_WIELDING
#define WLD_LOG_NVPS LOG_NVPS
#else
#define WLD_LOG_NVPS WLD_LOG
#endif

template <class E>
void perform_wielding(
	const logic_step step,
	const E& self,
	const wielding_setup& request,
	const bool should_play_effects = true
) {
	if (logically_empty(request)) {
		return;
	}

	auto& cosm = self.get_cosmos();

	for (const auto& h : request.hand_selections) {
		if (const auto& requested_item = cosm[h]) {
			if (requested_item.get_owning_transfer_capability() != self) {
				/* This is an invalid request. */
				return;
			}
		}
	}

	auto swap_wielded = [&](const bool play_effects_at_all = true) {
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

		if (play_effects_at_all) {
			result.play_effects_only_in_first();
		}
		else {
			for (auto& r : result.transfers) {
				r.params.play_transfer_sounds = false;
			}
		}

		for (auto& r : result.transfers) {
			r.params.play_transfer_particles = false;
		}

		result.apply(step);
	};

	const auto& selections = request.hand_selections;
	const auto current_selection = wielding_setup::from_current(self);

	const auto first_held = cosm[current_selection.hand_selections[0]];
	const auto second_held = cosm[current_selection.hand_selections[1]];

	const bool total_holster = first_held.dead() && second_held.dead();

	WLD_LOG_NVPS(cosm[selections[0]]);
	WLD_LOG_NVPS(cosm[selections[1]]);

	if (!total_holster && (first_held == selections[1] && second_held == selections[0])) {
		WLD_LOG("Required swapped. Swap.");
		swap_wielded();
		return;
	}

	/* if (current_selection == request) { */
	/* 	WLD_LOG("Same setup. Swap."); */
	/* 	swap_wielded(); */
	/* 	continue; */
	/* } */

#if 1
	if (request.is_akimbo(cosm)
		&& first_held == selections[1]
		&& second_held == entity_id()
	) {
		WLD_LOG("Move to secondary and draw.");

		wielding_result result;

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
		result.apply(step);
		return;
	}

	/* Swap now if we have to. */
	if (first_held == selections[1] || second_held == selections[0]) {
		swap_wielded(false);
	}

	thread_local std::vector<perform_transfer_result> results;
	results.clear();

	/* Now we can consider each hand separately. */
	auto transfer = [&](const auto& from, const auto& to) {
		if (from.dead()) {
			return;
		}

		auto request = item_slot_transfer_request::standard(from, to);
		request.params.play_transfer_particles = false;
		const auto result = perform_transfer_no_step(request, cosm);

		result.notify(step);
		results.push_back(result);
	};

	std::array<bool, hand_count_v> failed_hands = {};

	auto try_hand = [&](const std::size_t i) {
		failed_hands[i] = false;

		const auto hand = self.get_hand_no(i);

		const auto item_for_hand = cosm[selections[i]];
		const auto item_in_hand = hand.get_item_if_any();

		WLD_LOG_NVPS(i, item_for_hand, item_in_hand);

		const bool identical_outcome =
			(item_in_hand.dead() && item_for_hand.dead())
			|| item_in_hand == item_for_hand
		;

		if (identical_outcome) {
			WLD_LOG("Identical outcome.");
			return;
		}

		if (item_in_hand.alive()) {
			WLD_LOG("Has to holster existing item.");

			const auto holstering_slot = self.find_holstering_slot_for(item_in_hand);

			if (holstering_slot.alive()) {
				WLD_LOG("Holster found.");
				WLD_LOG_NVPS(holstering_slot);
				transfer(item_in_hand, holstering_slot);
				transfer(item_for_hand, hand);
			}
			else {
				WLD_LOG("Holster failed.");
				failed_hands[i] = true;
			}
		}
		else {
			WLD_LOG("No need to holster, draw right away.");
			transfer(item_for_hand, hand);
		}
	};

	for (std::size_t i = 0; i < hand_count_v; ++i) {
		try_hand(i);
	}

	for (std::size_t i = 0; i < hand_count_v; ++i) {
		const auto other_hand = self.get_hand_no(1 - i);

		if (failed_hands[i]) {
			WLD_LOG("Hand %x failed, so retry.", i);
			/* */
			try_hand(i);

			if (failed_hands[i]) {
				WLD_LOG("Hand %x still fails...", i);
				if (other_hand.is_empty_slot()) {
					const auto hand = self.get_hand_no(i);

					const auto item_in_hand = hand.get_item_if_any();
					const auto item_for_hand = cosm[selections[i]];
					
					transfer(item_in_hand, other_hand);
					transfer(item_for_hand, hand);
					WLD_LOG("Hand %x failed, but other is empty so switch the hands.", i);
					break;
				}
			}
		}
	}

	for (std::size_t i = 0; i < hand_count_v; ++i) {
		/* Finally, try to holster whatever can be holstered now. */
		try_hand(i);
	}

	if (should_play_effects) {
		bool wield_played = false;

		for (const auto& r : reverse(results)) {
			if (r.result.result.is_wield()) {
				r.play_effects(step);
				wield_played = true;
				break;
			}
		}

		if (!wield_played) {
			for (const auto& r : reverse(results)) {
				if (r.result.result.is_wear() || r.result.result.is_holster()) {
					r.play_effects(step);
				}
			}
		}
	}


#else
	/* Brute force approach: 
		1. Drop whatever is held.
		2. Draw required items.
		3. Holster dropped items.
	*/
#endif
}

