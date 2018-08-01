#pragma once
#include "game/cosmos/step_declaration.h"

class intent_contextualization_system {
public:
	void handle_use_button_presses(const logic_step);
	void advance_use_button(const logic_step);

	void contextualize_movement_intents(const logic_step step);
	void contextualize_use_button_intents(const logic_step step);
	void contextualize_crosshair_action_intents(const logic_step step);
};