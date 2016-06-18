#pragma once

#include <vector>

#include "math/vec2.h" 
#include "game/components/transform_component.h"

namespace components {
	struct melee {
		enum class state {
			FREE,
			ONCOOLDOWN,
			PRIMARY,
			SECONDARY,
			TERTIARY,
		};

		enum class stage {
			FIRST_STAGE,
			SECOND_STAGE,
			THIRD_STAGE,
			FOURTH_STAGE,
			WINDOW_STAGE //During the window stage the player can perform the second swing or an other melee action.
		};

		struct swing {
			float duration_ms;
			float acceleration;
			float cooldown_ms;
		};

		swing swings[5];
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

		void reset_weapon(entity_id e);

		state current_state = state::FREE;

		std::vector<components::transform> offset_positions[4];
	};
}