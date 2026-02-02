#pragma once

#include "game/cosmos/step_declaration.h"

class destruction_system {
public:
	void generate_damages_from_forceful_collisions(const logic_step) const;
	void generate_damages_for_pending_destructions(const logic_step) const;
	void apply_damages_and_split_fixtures(const logic_step) const;
};