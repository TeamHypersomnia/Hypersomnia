#pragma once
#include "game/detail/item_slot_transfer_request.h"

class cosmos;
class fixed_step;

class item_system {
public:
	void handle_trigger_confirmations_as_pick_requests(fixed_step& step);
	
	void handle_throw_item_intents(fixed_step& step);
	void handle_holster_item_intents(fixed_step& step);

	void translate_gui_intents_to_transfer_requests(fixed_step& step);
	
	void process_mounting_and_unmounting(fixed_step& step);
};