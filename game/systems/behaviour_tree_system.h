#pragma once

class physics_system;
class cosmos;
class fixed_step;

class behaviour_tree_system {
public:
	void evaluate_trees(cosmos& cosmos);
};