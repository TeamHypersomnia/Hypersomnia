#pragma once
#include "augs/math/vec2.h" 
#include "game/components/transform_component.h"
#include "game/enums/melee_state.h"

#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"
#include "padding_byte.h"

namespace components {
	struct melee {
		enum class stage {
			FIRST_STAGE,
			SECOND_STAGE,
			THIRD_STAGE,
			FOURTH_STAGE,
			WINDOW_STAGE //During the window stage the player can perform the second swing or an other melee action.
		};

		struct swing {
			float duration_ms = 0.f;
			float acceleration = 0.f;
			float cooldown_ms = 0.f;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(duration_ms),
					CEREAL_NVP(acceleration),
					CEREAL_NVP(cooldown_ms)
				);
			}
		};

		swing swings[5];
		float swing_current_time = 0.f;
		float swing_current_cooldown_time = 0.f;

		float window_time = 500.0f; //
		float window_current_time = 0.f;

		bool primary_move_flag = false;
		bool secondary_move_flag = false;
		bool tertiary_move_flag = false;
		padding_byte pad;

		melee_state current_state = melee_state::FREE;

		augs::constant_size_vector<components::transform, MELEE_OFFSET_POSITIONS_COUNT> offset_positions[4];
		stage action_stage = stage::FIRST_STAGE;

		void reset_move_flags() {
			primary_move_flag = secondary_move_flag = tertiary_move_flag = false;
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(swings),
				CEREAL_NVP(swing_current_time),
				CEREAL_NVP(swing_current_cooldown_time),

				CEREAL_NVP(window_time),
				CEREAL_NVP(window_current_time),

				CEREAL_NVP(primary_move_flag),
				CEREAL_NVP(secondary_move_flag),
				CEREAL_NVP(tertiary_move_flag),

				CEREAL_NVP(current_state),

				CEREAL_NVP(offset_positions),
				CEREAL_NVP(action_stage)
			);
		}

		void reset_weapon(entity_handle e);
	};
}