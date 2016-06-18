#pragma once
#include "game/processing_system_with_cosmos_reference.h"
#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"

using namespace augs;

class item_system {
public:
	void handle_trigger_confirmations_as_pick_requests();
	
	void handle_throw_item_intents();
	void handle_holster_item_intents();

	void translate_gui_intents_to_transfer_requests();
	static void consume_item_slot_transfer_requests();

	void process_mounting_and_unmounting();
};