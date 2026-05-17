#pragma once
#include "augs/enums/callback_result.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/logic_step.h"
#include "game/enums/slot_function.h"
#include "game/components/melee_component.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"

/*
	Ensure the SHOULDER slot always carries the largest melee that the subject
	owns (excluding what is currently in hand). If a smaller melee is mounted,
	swap it out to a deposit silently. No sounds, particles or recoils.
*/
template <class E>
void silently_fill_shoulder_with_largest_melee(const logic_step step, const E& typed_subject) {
	const auto shoulder_slot = typed_subject[slot_function::SHOULDER];

	if (shoulder_slot.dead()) {
		return;
	}

	auto& cosm = typed_subject.get_cosmos();

	entity_id best_knife;
	inventory_space_type best_space = 0;

	typed_subject.for_each_contained_item_recursive(
		[&](const auto& candidate_item) {
			if (candidate_item.get_current_slot().is_hand_slot()) {
				return recursive_callback_result::CONTINUE_AND_RECURSE;
			}

			if (candidate_item.template has<components::melee>()) {
				const auto space = ::calc_space_occupied_with_children(candidate_item);

				if (!best_knife.is_set() || space > best_space) {
					best_knife = candidate_item.get_id();
					best_space = space;
				}
			}

			return recursive_callback_result::CONTINUE_AND_RECURSE;
		}
	);

	if (!best_knife.is_set()) {
		return;
	}

	const auto knife_handle = cosm[best_knife];

	if (knife_handle.get_current_slot().get_id() == shoulder_slot.get_id()) {
		/* The largest melee is already mounted. */
		return;
	}

	auto silent_transfer = [&](const entity_id from, const auto& to) {
		auto request = item_slot_transfer_request::standard(from, to);
		request.params.play_transfer_sounds = false;
		request.params.play_transfer_particles = false;
		request.params.perform_recoils = false;
		const auto r = perform_transfer_no_step(request, cosm);
		r.notify(step);
	};

	if (const auto shoulder_item = shoulder_slot.get_item_if_any()) {
		if (!shoulder_item.template has<components::melee>()) {
			/* SHOULDER holds something else - leave it alone. */
			return;
		}

		const auto shoulder_space = ::calc_space_occupied_with_children(shoulder_item);

		if (best_space <= shoulder_space) {
			return;
		}

		const auto kick_destination = typed_subject.find_holstering_slot_for(shoulder_item);

		if (kick_destination.dead()) {
			return;
		}

		silent_transfer(entity_id(shoulder_item.get_id()), kick_destination);
	}

	if (shoulder_slot.can_contain(knife_handle)) {
		silent_transfer(entity_id(knife_handle.get_id()), shoulder_slot);
	}
}
