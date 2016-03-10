#pragma once
#include "entity_system/entity.h"
#include "../detail/inventory_slot_id.h"

namespace messages {
	struct gui_item_transfer_intent {
		augs::entity_id item;
		inventory_slot_id target_slot;
	};
}
