#pragma once
#include <vector>
#include "math/vec2.h"

class recoil_player {
	int delta_offset = 0;
public:
	std::vector<vec2> offsets;
	int current_offset = 0;
	bool reversed = false;
	int repeat_last_n_offsets = 5;
	// augs::deterministic_timestamp since_last_shot;

	double single_cooldown_duration_ms = 50.0;
	double remaining_cooldown_duration = -1.0;
	double scale = 1.0;

	vec2 shoot_and_get_offset();
	void cooldown(double amount_ms);
};
