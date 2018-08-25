#pragma once
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/enums/faction_type.h"
#include "game/cosmos/entity_flavour_id.h"

namespace components {
	struct sender {
		// GEN INTROSPECTOR struct components::sender
		signi_entity_id direct_sender;
		entity_flavour_id direct_sender_flavour;

		signi_entity_id capability_of_sender;
		signi_entity_id vehicle_driven_by_capability;
		faction_type faction_of_sender = faction_type::NONE;
		// END GEN INTROSPECTOR

		void set(const const_entity_handle direct_sender);
		void set_direct(const const_entity_handle direct_sender);

		bool is_sender_subject(const const_entity_handle potential_sender) const;
	};
}