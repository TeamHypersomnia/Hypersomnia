#pragma once

class cosmos;
class logic_step;

class force_joint_system {
public:

	void apply_forces_towards_target_entities(logic_step& step);
};