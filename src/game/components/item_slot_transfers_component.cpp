#include "item_slot_transfers_component.h"
#include "item_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

namespace components {
#if TODO_MOUNTING
	item_slot_mounting_operation item_slot_transfers::find_suitable_montage_operation(const_entity_handle parent_container) {
		ensure(false);
		item_slot_mounting_operation operation;

		return operation;
		//auto* item = parent_container.find<components::item>();
		//auto* container = parent_container.find<invariants::container>();
		//
		//if (item) {
		//	if (item->intended_mounting != item->current_mounting) {
		//		operation.current_item = parent_container;
		//		operation.intented_mounting_slot = item->get_current_slot();
		//
		//		return operation;
		//	}
		//}
		//else if (container) {
		//	for (auto& s : container->slots) {
			//		for (auto& i : s.second.get_items_inside()) {
		//			operation = find_suitable_montage_operation(parent_container.get_cosmos()[i]);
		//			
		//			if (operation.alive())
		//				return operation;
		//		}
		//	}
		//}
		//
		//return operation;
	}
#endif
}