#pragma once
#include <vector>
#include "math/vec2.h"
#include "game/entity_handle_declaration.h"

class recoil_player {
	int delta_offset = 0;
public:
	std::vector<vec2> offsets;
	int current_offset = 0;
	bool reversed = false;
	int repeat_last_n_offsets = 5;

	double single_cooldown_duration_ms = 50.0;
	double remaining_cooldown_duration = -1.0;
	double scale = 1.0;

	vec2 shoot_and_get_offset();
	
	void shoot_and_apply_impulse(entity_handle recoil_body, float scale, bool angular_impulse = false, float additional_angle = 0.f,
		bool positional_impulse = false, float positional_rotation = 0.f);
	
	void cooldown(double amount_ms);
};
