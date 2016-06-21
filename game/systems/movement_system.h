#pragma once

class cosmos;
class step_state;

class movement_system {
public:

	void set_movement_flags_from_input(cosmos& cosmos, step_state& step);
	void apply_movement_forces(cosmos& cosmos);
	void generate_movement_responses(cosmos& cosmos, step_state& step);
};