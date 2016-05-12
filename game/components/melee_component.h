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

	struct melee_info {
		float duration_ms;
		float acceleration;
		float cooldown_ms;
	};

	struct melee {
		melee_info swings[5];
		float swing_current_time = 0.f;
		float swing_current_cooldown_time = 0.f;

		float window_time = 500.0f; //
		float window_current_time = 0.f;

		bool primary_move_flag = false;
		bool secondary_move_flag = false;
		bool tertiary_move_flag = false;

		void reset_move_flags() {
			primary_move_flag = secondary_move_flag = tertiary_move_flag = false;
		}

		melee_state state = MELEE_FREE;

		std::vector<components::transform> offset_positions[4];
	};
}