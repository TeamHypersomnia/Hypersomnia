#include "game/cosmos/cosmos_global_solvable.h"
#include "augs/templates/container_templates.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/components/item_sync.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/inventory/item_mounting.hpp"

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

		const auto item_handle = cosm[e_id];

		if (item_handle.dead()) {
			return true;
		}

		bool should_be_erased = true;

		item_handle.template dispatch_on_having_all<components::item>([&](const auto& transferred_item) {
			auto& request = m.second;
			auto& specified_charges = request.params.specified_quantity;

			if (specified_charges == 0) {
				return;
			}

			auto& progress = request.progress_ms;

			const auto target_slot = cosm[request.target];
			const auto conditions = calc_mounting_conditions(transferred_item, target_slot);

			if (conditions == mounting_conditions_type::PROGRESS) {
				should_be_erased = false;
				progress += delta.in_milliseconds();

				const auto considered_max = request.get_mounting_duration_ms(transferred_item); 

				if (progress >= considered_max) {
					item_slot_transfer_request transfer;
					transfer.item = transferred_item;
					transfer.target_slot = target_slot;
					transfer.params = request.params;
					transfer.params.bypass_mounting_requirements = true;
					transfer.params.specified_quantity = 1;

					const auto previous_charges = transferred_item.template get<components::item>().get_charges();

					::perform_transfer(transfer, step);

					if (previous_charges == 1) {
						should_be_erased = true;
						return;
					}

					if (specified_charges != -1) {
						--specified_charges;
						progress = 0.f;

						if (specified_charges == 0) {
							should_be_erased = true;
							return;
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

