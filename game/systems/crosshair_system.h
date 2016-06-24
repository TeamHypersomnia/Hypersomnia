#pragma once
class cosmos;
class fixed_step;

class crosshair_system {
public:
	void generate_crosshair_intents(fixed_step&);
	
	void apply_crosshair_intents_to_base_offsets(fixed_step&);
	void apply_base_offsets_to_crosshair_transforms(fixed_step&);

	void animate_crosshair_sizes(cosmos&);
};