#include "step_state.h"
#include "game/systems/item_system.h"

step_state::step_state() {
	messages.register_callback<messages::item_slot_transfer_request>(item_system::consume_item_slot_transfer_requests);
}