#pragma once
class cosmos;
class step_state;

class crosshair_system {
public:
	void generate_crosshair_intents(step_state&);
	
	void apply_crosshair_intents_to_base_offsets(cosmos&, step_state&);
	void apply_base_offsets_to_crosshair_transforms(cosmos&, step_state&);

	void animate_crosshair_sizes(cosmos&);
};