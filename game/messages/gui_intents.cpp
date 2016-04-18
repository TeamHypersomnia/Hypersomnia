#include "gui_intents.h"
#include "../detail/inventory_slot_id.h"
#include "item_slot_transfer_request.h"

namespace messages {
	gui_item_transfer_intent& gui_item_transfer_intent::operator=(const messages::item_slot_transfer_request& b) {
		item = b.item;
		target_slot = b.target_slot;
		specified_quantity = b.specified_quantity;
		return *this;
	}
}