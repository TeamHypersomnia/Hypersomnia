#include "gui_intents.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/item_slot_transfer_request.h"

namespace messages {
	gui_item_transfer_intent& gui_item_transfer_intent::operator=(const item_slot_transfer_request& b) {
		item = b.item;
		target_slot = b.target_slot;
		specified_quantity = b.specified_quantity;
		return *this;
	}
}