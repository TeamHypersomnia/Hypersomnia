#pragma once

class cosmos;
#include "game/transcendental/step_declaration.h"

class force_joint_system {
public:

	void apply_forces_towards_target_entities(logic_step& step);
};