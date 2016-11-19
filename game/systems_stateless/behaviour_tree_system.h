#pragma once
#include "game/transcendental/step_declaration.h"

class physics_system;
class cosmos;

class behaviour_tree_system {
public:
	void evaluate_trees(logic_step& cosmos);
};