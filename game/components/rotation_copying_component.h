#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"
#include "padding_byte.h"

namespace components {
	struct rotation_copying {
		enum class look_type {
			POSITION,
			VELOCITY,
			ROTATION
		};

		enum class easing_type {
			NONE,
			LINEAR,
			EXPONENTIAL
		};
		
		entity_id target;
		entity_id stashed_target;

		easing_type easing_mode = easing_type::NONE;

		bool colinearize_item_in_hand = false;
		bool update_value = true;
		padding_byte pad[2];
		
		/* for exponential smoothing */
		float smoothing_average_factor = 0.5f;
		float averages_per_sec = 20.0f;
		
		/* for linear smoothing */
		vec2 last_rotation_interpolant;

		look_type look_mode = look_type::POSITION;
		look_type stashed_look_mode = look_type::POSITION;

		void stash() {
			stashed_look_mode = look_mode;
			stashed_target = target;
		}

		void unstash() {
			look_mode = stashed_look_mode;
			target = stashed_target;
		}

		template<class F>
		void for_each_held_id(F f) {
			f(target);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(target);
		}
		
		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(target),

				CEREAL_NVP(easing_mode),
				CEREAL_NVP(colinearize_item_in_hand),

				CEREAL_NVP(smoothing_average_factor),
				CEREAL_NVP(averages_per_sec),

				CEREAL_NVP(last_rotation_interpolant),

				CEREAL_NVP(look_mode),

				CEREAL_NVP(last_value),
				CEREAL_NVP(update_value),

				CEREAL_NVP(use_physical_motor)
			);
		}
	};
}