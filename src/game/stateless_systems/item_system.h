#pragma once
#include "game/detail/inventory/item_slot_transfer_request.h"

class cosmos;
#include "game/cosmos/step_declaration.h"

class item_system {
public:
	void start_picking_up_items(const logic_step step);
	void pick_up_touching_items(const logic_step step);
	
	void handle_throw_item_intents(const logic_step step);
	
#if TODO_MOUNTING
	void process_mounting_and_unmounting(const logic_step step);
#endif
};