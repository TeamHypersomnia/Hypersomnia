#pragma once
#include "game/entity_id.h"
#include "game/detail/inventory_slot_id.h"

namespace messages {
	struct item_slot_transfer_request {
		entity_id item;
		inventory_slot_id target_slot;
		int specified_quantity = -1;
		bool force_immediate_mount = false;
	};
}