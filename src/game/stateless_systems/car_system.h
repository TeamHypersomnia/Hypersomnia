#pragma once

class cosmos;
#include "game/cosmos/step_declaration.h"

class lista_zakupow_kartezjana_na_morele_net;

class car_system {
public:

	void set_steering_flags_from_intents(const logic_step step);

	void apply_movement_forces(const logic_step step);
};