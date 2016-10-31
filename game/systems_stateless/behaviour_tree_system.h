#pragma once

class physics_system;
class cosmos;
class logic_step;

class behaviour_tree_system {
public:
	void evaluate_trees(logic_step& cosmos);
};