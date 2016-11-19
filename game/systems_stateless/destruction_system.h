#pragma once

#include "game/transcendental/step_declaration.h"

class destruction_system {
public:
	void generate_damages_from_forceful_collisions(logic_step&) const;
	void apply_damages_and_split_fixtures(logic_step&) const;
};