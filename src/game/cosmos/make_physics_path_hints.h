#pragma once
#include "game/detail/physics_path_hints.h"
#include "game/cosmos/cosmos.h"
#include "game/enums/filters.h"

/*
	Helper function to create physics_path_hints from a cosmos.
	Uses the pathfinding filter for line-of-sight checks.
*/

inline physics_path_hints make_physics_path_hints(const cosmos& cosm) {
	return physics_path_hints{
		cosm.get_solvable_inferred().physics,
		cosm.get_si(),
		predefined_queries::pathfinding()
	};
}
