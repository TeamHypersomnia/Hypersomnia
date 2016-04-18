#pragma once
#include <vector>
#include "entity_system/entity_id.h"
#include "../detail/inventory_slot_id.h"

#include "misc/deterministic_timing.h"

namespace components {
	struct item_slot_transfers {
		struct mounting_operation {
			augs::entity_id current_item;
			inventory_slot_id intented_mounting_slot;

			bool alive() const { 
				return current_item.alive(); 
			}
		} mounting;

		augs::deterministic_timeout pickup_timeout = augs::deterministic_timeout(200);

		static mounting_operation find_suitable_montage_operation(augs::entity_id parent_container);

		void interrupt_mounting();
	};
}