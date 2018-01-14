#pragma once
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct sender {
		/*
			We are only interested in the vehicle driven by capability at the time of sending.
		*/
		
		// GEN INTROSPECTOR struct components::sender
		entity_id direct_sender;
		entity_id capability_of_sender;
		entity_id vehicle_driven_by_capability;
		// END GEN INTROSPECTOR

		void set(const const_entity_handle direct_sender);
		bool is_sender_subject(const const_entity_handle potential_sender) const;
	};
}