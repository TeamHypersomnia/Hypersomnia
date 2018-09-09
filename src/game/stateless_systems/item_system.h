#pragma once
#include "game/detail/inventory/item_slot_transfer_request.h"

class cosmos;
#include "game/cosmos/step_declaration.h"

class item_system {
public:
	void pick_up_touching_items(const logic_step step);
	void handle_throw_item_intents(const logic_step step);
};