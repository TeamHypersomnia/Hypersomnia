#pragma once
#include "game/cosmos/step_declaration.h"

class demolitions_system {
public:
	void handle_arming_requests(const logic_step step);
	void detonate_fuses(const logic_step step);
	void advance_cascade_explosions(const logic_step step);
};
