#pragma once
#include "game/cosmos/step_declaration.h"

class physics_world_cache;
class cosmos;

class behaviour_tree_system {
public:
	void evaluate_trees(const logic_step cosm);
};