#pragma once

#include <vector>

#include "math/vec2.h" 

namespace components {
	enum melee_state {
		MELEE_FREE,
		MELEE_ONCOOLDOWN,
		MELEE_PRIMARY,
		MELEE_SECONDARY,
		MELEE_TERTIARY,
	};
	struct melee {

		float swing_duration_ms = 200.f;
		float swing_cooldown_ms = 100.f;
		float swing_current_time = 0.f;
		float swing_current_cooldown_time = 0.f;
		float swing_acceleration = 5.0f;

		bool primary_move_flag = false;
		bool secondary_move_flag = false;
		bool tertiary_move_flag = false;

		melee_state state = MELEE_FREE; 

		std::vector<vec2> offset_positions;
	};
}