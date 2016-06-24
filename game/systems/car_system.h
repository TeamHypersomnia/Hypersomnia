#pragma once

class cosmos;
class step_state;

class lista_zakupow_kartezjana_na_morele_net;

class car_system {
public:

	void set_steering_flags_from_intents(fixed_step& step);

	void apply_movement_forces(fixed_step& step);
};