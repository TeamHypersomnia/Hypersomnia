#pragma once

class cosmos;
class step_state;

class force_joint_system {
public:

	void apply_forces_towards_target_entities(fixed_step& step);
};