#pragma once
#include <vector>
#include "math/vec2.h"
#include "deterministic_timing.h"

class recoil_player {
public:
	std::vector<vec2> offsets;
	int current_offset = 0;

	augs::deterministic_timestamp since_last_shot;

	double single_cooldown_duration_ms = 100.0;
	double scale = 1.f;

	vec2 shoot_and_get_offset(augs::deterministic_timestamp current_time);
};