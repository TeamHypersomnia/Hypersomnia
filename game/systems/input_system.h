#pragma once
class cosmos;
class fixed_step;

namespace augs {
	struct machine_entropy;
}

struct input_system {
	void post_unmapped_intents_from_raw_window_inputs(fixed_step&);
	void map_unmapped_intents_to_entities(fixed_step&);
};