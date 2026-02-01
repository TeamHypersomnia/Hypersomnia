#pragma once
#include "game/detail/inventory/item_mounting.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id.h"
#include "augs/math/vec2.h"

struct pending_destruction {
	// GEN INTROSPECTOR struct pending_destruction
	entity_id target;
	real32 delay_ms = 0.0f;
	vec2 impact_velocity = vec2::zero;
	// END GEN INTROSPECTOR
};

using pending_destructions_type = std::vector<pending_destruction>;

struct cosmos_global_solvable {
	// GEN INTROSPECTOR struct cosmos_global_solvable
	pending_item_mounts_type pending_item_mounts;
	pending_destructions_type pending_destructions;
	// END GEN INTROSPECTOR

	void solve_item_mounting(logic_step);
	void clear();
};
