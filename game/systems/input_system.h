#pragma once
class cosmos;
class fixed_step;

namespace augs {
	struct machine_entropy;
}

struct input_system {
	void post_unmapped_intents_from_raw_window_inputs(cosmos&, step_state&, const machine_entropy&);
	void map_unmapped_intents_to_entities(cosmos&, step_state&);
};