#include "game/cosmos/cosmos_global_solvable.h"
#include "augs/templates/container_templates.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/components/item_sync.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"

real32 pending_item_mount::get_mounting_duration_ms(const const_entity_handle& handle) const {
	if (const auto current_slot = handle.get_current_slot()) {
		const auto& cosm = handle.get_cosmos();

		const auto target_slot = cosm[target];

		if (const bool source_mounted = current_slot->is_mounted_slot()) {
			/* We're unmounting */
			auto d = current_slot->mounting_duration_ms;

			if (target_slot.dead()) {
				/* We're dropping, so twice as fast */
				d *= 0.5f;
			}

			return d;
		}
		else if (target_slot) {
			/* We're mounting */
			return target_slot->mounting_duration_ms;
		}
	}

	return -1.f;
}

void cosmos_global_solvable::solve_item_mounting(const logic_step step) {
	const auto delta = step.get_delta();
	auto& cosm = step.get_cosmos();

	erase_if(pending_item_mounts, [&](auto& m) {
		const auto& e_id = m.first;

		bool should_be_erased = true;

		cosm[e_id].template dispatch_on_having_all<components::item>([&](const auto& e) {
			auto& request = m.second;
			auto& progress = request.progress_ms;

			if (const auto current_slot = e.get_current_slot()) {
				const auto& target_mount = cosm[request.target];

				const bool source_mounted = current_slot->is_mounted_slot();
				const bool target_mounted = target_mount.alive() ? target_mount->is_mounted_slot() : false;

				if (source_mounted != target_mounted) {
					/* (Un)mounting was requested */
					if (const auto capability = e.get_owning_transfer_capability()) {
						const bool both_in_reach_of_hands = [&]() {
							const auto reachable_non_physical = augs::enum_boolset<slot_function> {
								slot_function::GUN_CHAMBER,
								slot_function::GUN_DETACHABLE_MAGAZINE,
								slot_function::GUN_RAIL,
								slot_function::GUN_MUZZLE,
								slot_function::GUN_CHAMBER_MAGAZINE
							};

							if (!target_mounted) {
								/* We're unmounting. Source must be physically connected to hands. */
								return current_slot.is_physically_connected_until(capability, reachable_non_physical);
							}

							/* We're mounting. Source must be directly in hands, and target must be physically connected to hands. */
							return current_slot.is_hand_slot() && target_mount.is_physically_connected_until(capability, reachable_non_physical);
						}();

						if (both_in_reach_of_hands) {
							should_be_erased = false;
							progress += delta.in_milliseconds();

							const auto considered_max = request.get_mounting_duration_ms(e); 

							if (progress >= considered_max) {
								item_slot_transfer_request request;
								request.params.specified_quantity = 1;
								request.item = e;
								request.target_slot = target_mount;
								request.params.bypass_mounting_requirements = true;

								::perform_transfer(request, step);

								should_be_erased = true;
							}
						}
					}
				}
			}
		});

		return should_be_erased;
	});
}

void cosmos_global_solvable::clear() {
	pending_item_mounts.clear();
}

