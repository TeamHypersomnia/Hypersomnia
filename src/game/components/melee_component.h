#pragma once
#include "augs/math/vec2.h" 
#include "game/components/transform_component.h"
#include "game/enums/melee_state.h"

#include "game/container_sizes.h"
#include "augs/pad_bytes.h"

namespace components {
	struct melee {
		enum class stage {
			FIRST_STAGE,
			SECOND_STAGE,
			THIRD_STAGE,
			FOURTH_STAGE,
			WINDOW_STAGE //During the window stage the player can perform the second swing or an other melee action.
		};

		// GEN INTROSPECTOR struct components::melee
		bool primary_move_flag = false;
		bool secondary_move_flag = false;
		bool tertiary_move_flag = false;
		pad_bytes<1> pad;

		melee_state current_state = melee_state::FREE;
		// END GEN INTROSPECTOR

		void reset_move_flags() {
			primary_move_flag = secondary_move_flag = tertiary_move_flag = false;
		}

		void reset_weapon(entity_handle e);
	};
}