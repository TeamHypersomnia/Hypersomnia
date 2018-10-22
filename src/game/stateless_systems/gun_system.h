#pragma once

class physics_world_cache;
class cosmos;
#include "game/cosmos/step_declaration.h"

class gun_system {
public:
	void launch_shots_due_to_pressed_triggers(const logic_step step);
};