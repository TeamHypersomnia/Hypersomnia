#pragma once
#include <vector>
#include "game/entity_id.h"
#include "game/detail/inventory_slot_id.h"

#include "misc/stepped_timing.h"
#include <set>

namespace components {
	struct item_slot_transfers {
		struct mounting_operation {
			entity_id current_item;
			inventory_slot_id intented_mounting_slot;
		} mounting;

		std::set<entity_id> only_pick_these_items;
		bool pick_all_touched_items_if_list_to_pick_empty = true;

		augs::deterministic_timeout pickup_timeout = augs::deterministic_timeout(200);

		static mounting_operation find_suitable_montage_operation(entity_id parent_container);

		void interrupt_mounting();
	};
}