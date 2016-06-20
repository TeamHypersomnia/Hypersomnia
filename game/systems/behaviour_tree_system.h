#pragma once

class physics_system;
class cosmos;
class step_state;

class behaviour_tree_system {
public:
	void evaluate_trees(cosmos& cosmos, step_state& step);
};