#include "item_slot_transfers_component.h"
#include "item_component.h"
#include "game/cosmos.h"

namespace components {
	item_slot_transfers::mounting_operation item_slot_transfers::find_suitable_montage_operation(entity_id parent_container) {
		item_slot_transfers::mounting_operation operation;

		auto* item = parent_container->find<components::item>();
		auto* container = parent_container->find<components::container>();

		if (item) {
			if (item->intended_mounting != item->current_mounting) {
				operation.current_item = parent_container;
				operation.intented_mounting_slot = item->current_slot;

				return operation;
			}
		}
		else if (container) {
			for (auto& s : container->slots) {
				for (auto& i : s.second.items_inside) {
					operation = find_suitable_montage_operation(i);
					
					if (operation.alive())
						return operation;
				}
			}
		}
		
		return operation;
	}
}