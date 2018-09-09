#pragma once
#include "game/detail/inventory/item_mounting.h"
#include "game/cosmos/step_declaration.h"

struct cosmos_global_solvable {
	// GEN INTROSPECTOR struct cosmos_global_solvable
	pending_item_mounts_type pending_item_mounts;
	// END GEN INTROSPECTOR

	void solve_item_mounting(logic_step);
	void clear();
};

