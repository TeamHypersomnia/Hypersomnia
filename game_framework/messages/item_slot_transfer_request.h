#pragma once
#include "entity_system/entity_id.h"
#include "../shared/inventory_slot_id.h"

namespace messages {
	struct item_slot_transfer_intent {
		augs::entity_id item;
		inventory_slot_id target_slot;
	};

	struct item_slot_transfer_request {
		augs::entity_id item;
		inventory_slot_id target_slot;
		item_transfer_result intent_result = item_transfer_result::SUCCESSFUL_TRANSFER;
	};
}