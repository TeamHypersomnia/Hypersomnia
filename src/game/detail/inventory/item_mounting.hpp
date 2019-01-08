#pragma once

enum class mounting_conditions_type {
	ABORT,
	PROGRESS,
	NO_MOUNTING_REQUIRED
};

template <class A, class B>
mounting_conditions_type calc_mounting_conditions(
	const A& item_entity,
	const B& target_slot
) {
	auto& cosm = item_entity.get_cosmos();

	const auto source_slot = item_entity.alive() ? item_entity.get_current_slot() : cosm[inventory_slot_id()];

	const auto source_capability = item_entity.get_owning_transfer_capability();

	if (source_capability.dead()) {
		/* 
			We can't mount or unmount an item that has no capability (so also if it has no slot). 
			First we have to hold it in our hands. 
		*/

		if (target_slot.is_this_or_ancestor_mounted()) {
			/* Target slot can't be mounted or have a mounted ancestor. */
			return mounting_conditions_type::ABORT;
		}

		return mounting_conditions_type::NO_MOUNTING_REQUIRED;
	}

	const auto target_capability = target_slot.get_container().get_owning_transfer_capability();

	const auto& capability = source_capability;

	const bool source_mounted = source_slot->is_mounted_slot();
	const bool target_mounted = target_slot.alive() ? target_slot->is_mounted_slot() : false;

	if (source_mounted && target_mounted) {
		return mounting_conditions_type::ABORT;
	}

	const auto is_reachable = [&](const auto& slot, const bool for_mounting) {
		auto reachable_non_physical = slot_flags {
			slot_function::GUN_DETACHABLE_MAGAZINE,
			slot_function::GUN_RAIL,
			slot_function::GUN_MUZZLE,
			slot_function::GUN_CHAMBER_MAGAZINE,
			slot_function::TORSO_ARMOR
		};

		if (const bool for_unmounting = !for_mounting) {
			/* Always allow to unmount from the chamber. */
			reachable_non_physical.set(slot_function::GUN_CHAMBER);
		}

		return slot.is_physically_reachable_from(capability, reachable_non_physical);
	};

	const bool unmounting = source_mounted && !target_mounted;
	const bool mounting = !source_mounted && target_mounted;

	auto progress_if = [&](const bool flag) {
		return flag ? mounting_conditions_type::PROGRESS : mounting_conditions_type::ABORT;
	};

	if (mounting) {
		const bool viable = source_capability.alive() && target_capability == source_capability;

		if (!viable) {
			return mounting_conditions_type::ABORT;
		}

		if (!source_slot.is_hand_slot()) {
			/* Source item must be directly in hand to perform mount */
			return mounting_conditions_type::ABORT;
		}

		/* Target must be reachable by the capability */
		return progress_if(is_reachable(target_slot, true));
	}

	if (unmounting) {
		const bool viable = 
			source_capability.alive() && 
			(target_capability == source_capability || target_capability.dead())
		;

		if (!viable) {
			return mounting_conditions_type::ABORT;
		}

		const bool target_is_valid = [&]() {
			if (target_slot.dead()) {
				/* If the target is dead, one of the hands must be free */
				return capability.get_wielded_items().size() <= 1;
			}

			/* If the target is alive, it must be a free hand of the capability */
			return 
				target_slot.is_empty_slot() 
				&& (target_slot.get_id() == capability.get_primary_hand().get_id() || target_slot.get_id() == capability.get_secondary_hand().get_id())
			;
		}();

		return progress_if(is_reachable(source_slot, false) && target_is_valid);
	}

	/* 
		Both slots are non-mounted slots, but they might have mounted ancestors. 
		Any slot that has a mounted ancestor must be reachable by the capability for this transfer.
	*/

	if (source_slot.is_ancestor_mounted()) {
		if (!is_reachable(source_slot, false)) {
			return mounting_conditions_type::ABORT;
		}
	}

	if (target_slot.is_ancestor_mounted()) {
		if (target_slot.alive() && !is_reachable(target_slot, true)) {
			return mounting_conditions_type::ABORT;
		}
	}

	return mounting_conditions_type::NO_MOUNTING_REQUIRED;
}
