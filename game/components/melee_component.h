#pragma once

#include <vector>

#include "math/vec2.h" 
#include "game/components/transform_component.h"

namespace components {
	enum melee_state {
		MELEE_FREE,
		MELEE_ONCOOLDOWN,
		MELEE_PRIMARY,
		MELEE_SECONDARY,
		MELEE_TERTIARY,
	};
	struct melee {

		float swing_duration_ms[5];
		float swing_acceleration[4];
		float swing_cooldown_ms[4];
		float swing_current_time = 0.f;
		float swing_current_cooldown_time = 0.f;

		float window_time = 1000.0f; //
		float window_current_time = 0.f;

		bool primary_move_flag = false;
		bool secondary_move_flag = false;
		bool tertiary_move_flag = false;

		melee_state state = MELEE_FREE; 

		std::vector<components::transform> offset_positions[4];
	};
}