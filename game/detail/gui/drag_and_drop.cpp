#include "drag_and_drop.h"
#include "slot_button.h"
#include "item_button.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/components/item_component.h"
#include "game/detail/inventory_utils.h"
#include "game/transcendental/entity_id.h"
#include "augs/ensure.h"

bool drag_and_drop_result::will_drop_be_successful() {
	return result.result >= item_transfer_result_type::SUCCESSFUL_TRANSFER;
}

bool drag_and_drop_result::will_item_be_disposed() {
	return result.result >= item_transfer_result_type::SUCCESSFUL_TRANSFER && !target_slot_alive;
}