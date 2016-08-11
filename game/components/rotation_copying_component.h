#pragma once

#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"

namespace components {
	struct rotation_copying  {
		enum look_type {
			POSITION,
			VELOCITY,
		};

		enum rotation_copying_easing {
			NONE,
			LINEAR,
			EXPONENTIAL
		};
		
		entity_id target;

		int easing_mode = NONE;
		bool colinearize_item_in_hand = false;
		
		/* for exponential smoothing */
		double smoothing_average_factor = 0.5;
		double averages_per_sec = 20.0;
		
		/* for linear smoothing */
		vec2 last_rotation_interpolant;

		unsigned look_mode = look_type::POSITION;

		float last_value = 0.0f;
		bool update_value = true;

		bool use_physical_motor = false;

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