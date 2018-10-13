#pragma once
class cosmos;
#include "game/cosmos/step_declaration.h"

class crosshair_system {
public:
	void handle_crosshair_intents(const logic_step);
	void update_base_offsets(const logic_step);

	void integrate_crosshair_recoils(const logic_step);
};