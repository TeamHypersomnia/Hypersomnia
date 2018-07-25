#pragma once
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"

namespace components {
	struct sender {
		/*
			We are only interested in the vehicle driven by capability at the time of sending.
		*/
		
		// GEN INTROSPECTOR struct components::sender
		signi_entity_id direct_sender;
		signi_entity_id capability_of_sender;
		signi_entity_id vehicle_driven_by_capability;
		// END GEN INTROSPECTOR

		void set(const const_entity_handle direct_sender);
		bool is_sender_subject(const const_entity_handle potential_sender) const;
	};
}