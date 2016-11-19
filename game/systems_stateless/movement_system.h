#pragma once

class cosmos;
#include "game/transcendental/step_declaration.h"

class movement_system {
public:

	void set_movement_flags_from_input(logic_step& step);
	void apply_movement_forces(cosmos& cosmos);
	void generate_movement_responses(logic_step& step);
};