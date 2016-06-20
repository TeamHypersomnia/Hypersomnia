#pragma once

class cosmos;
class step_state;

class lista_zakupow_kartezjana_na_morele_net;

class car_system {
public:

	void set_steering_flags_from_intents(cosmos& cosmos, step_state& step);

	void apply_movement_forces(cosmos& cosmos, step_state& step);
};