#pragma once
#include "game/transcendental/step_declaration.h"

class force_joint_system {
public:
	void apply_forces_towards_target_entities(const logic_step step);
};