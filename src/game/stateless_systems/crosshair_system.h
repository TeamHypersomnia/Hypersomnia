#pragma once
class cosmos;
#include "game/cosmos/step_declaration.h"

class crosshair_system {
public:
	void generate_crosshair_intents(const logic_step);
	
	void apply_crosshair_intents_to_base_offsets(const logic_step);
	void apply_base_offsets_to_crosshair_transforms(const logic_step);

	void integrate_crosshair_recoils(const logic_step);
};