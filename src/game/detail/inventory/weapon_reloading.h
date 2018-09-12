#pragma once
#include <array>
#include "game/cosmos/entity_id.h"

struct reloading_context {
	// GEN INTROSPECTOR struct reloading_context
	std::array<signi_entity_id, 2> initial_setup;
	signi_inventory_slot_id concerned_slot;
	signi_entity_id new_ammo_source;
	signi_entity_id old_ammo_source;
	// END GEN INTROSPECTOR

	bool significantly_different_from(const reloading_context& b) const;
	bool is_chambering() const;
};
