#pragma once

class cosmos;
#include "game/transcendental/step_declaration.h"

class position_copying_system {
public:
	void update_transforms(const logic_step step);
};