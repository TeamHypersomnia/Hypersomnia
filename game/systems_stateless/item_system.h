#pragma once
#include "game/detail/item_slot_transfer_request.h"

class cosmos;
#include "game/transcendental/step_declaration.h"

class item_system {
public:
	void handle_trigger_confirmations_as_pick_requests(logic_step& step);
	
	void handle_throw_item_intents(logic_step& step);
	void handle_holster_item_intents(logic_step& step);
	
	void process_mounting_and_unmounting(logic_step& step);
};