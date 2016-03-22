#pragma once
#include "entity_system/processing_system.h"
#include "../components/item_component.h"
#include "../components/container_component.h"
#include "../components/item_slot_transfers_component.h"

using namespace augs;

class item_system : public augs::processing_system_templated<components::item_slot_transfers> {
public:

	using processing_system_templated::processing_system_templated;
	
	void handle_trigger_confirmations_as_pick_requests();
	
	void handle_throw_item_intents();
	void handle_holster_item_intents();

	void translate_gui_intents_to_transfer_requests();
	void consume_item_slot_transfer_requests();

	void process_mounting_and_unmounting();
};