#pragma once
#include <array>
#include "game/cosmos/entity_id.h"

struct reloading_context {
	// GEN INTROSPECTOR struct reloading_context
	signi_entity_id target_weapon;
	std::array<signi_entity_id, 2> initial_setup;
	signi_entity_id ammo_source;
	// END GEN INTROSPECTOR

	bool equivalent_to(const reloading_context& b) const {
		return target_weapon == b.target_weapon && ammo_source == b.ammo_source;
	}

	bool is_chambering() const {
		return !ammo_source.is_set();
	}
};
