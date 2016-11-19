#pragma once

class cosmos;
#include "game/transcendental/step_declaration.h"

class lista_zakupow_kartezjana_na_morele_net;

class car_system {
public:

	void set_steering_flags_from_intents(logic_step& step);

	void apply_movement_forces(logic_step& step);
};