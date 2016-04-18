#pragma once
#include "entity_system/entity.h"
#include "../detail/inventory_slot_id.h"

namespace messages {
	struct item_slot_transfer_request;
	struct gui_item_transfer_intent {
		augs::entity_id item;
		inventory_slot_id target_slot;
		int specified_quantity = -1;

		gui_item_transfer_intent& operator=(const messages::item_slot_transfer_request& b);
	};
}
