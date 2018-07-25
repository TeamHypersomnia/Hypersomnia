#pragma once
#include "game/cosmos/step_declaration.h"

class physics_system {
public:
	void step_and_set_new_transforms(const logic_step);
	void post_and_clear_accumulated_collision_messages(const logic_step);
};
