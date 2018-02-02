#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/pool/pool.h"

#include "game/transcendental/cosmic_types.h"
#include "game/transcendental/cosmos_clock.h"

struct cosmos_solvable_significant {
	// GEN INTROSPECTOR struct cosmos_solvable_significant
	all_aggregate_pools aggregate_pools;
	cosmos_clock clock;
	// END GEN INTROSPECTOR

	void clear();
};