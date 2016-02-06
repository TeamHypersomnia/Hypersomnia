#pragma once
#include "entity_system/processing_system.h"
#include "../components/item_component.h"
#include "../components/container_component.h"

using namespace augs;

class item_system : public augs::processing_system_templated<components::container> {
	bool post_holster_request(augs::entity_id item, bool drop_if_hiding_failed);

public:
	using processing_system_templated::processing_system_templated;
	
	void handle_trigger_confirmations_as_pick_requests();
	
	void handle_drop_item_requests();
	void handle_holster_item_requests();


	void process_pending_slot_item_transfers();
};