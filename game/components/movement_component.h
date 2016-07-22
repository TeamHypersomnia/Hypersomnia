#pragma once
#include "augs/math/vec2.h"
#include "game/entity_id.h"

namespace components {
	struct movement  {
		struct subscribtion {
			entity_id target;
			bool stop_response_at_zero_speed = false;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(target),
					CEREAL_NVP(stop_response_at_zero_speed)
				);
			}
		};

		bool apply_movement_forces = true;

		std::vector<subscribtion> response_receivers;
		
		bool moving_left = false;
		bool moving_right = false;
		bool moving_forward = false;
		bool moving_backward = false;
		bool walking_enabled = false;
		
		vec2 input_acceleration_axes;
		float acceleration_length = -1.f;

		vec2 applied_force_offset;

		float non_braking_damping = 0.f;
		float braking_damping = 0.f;

		float standard_linear_damping = 0.f;
		
		bool enable_braking_damping = false;

		bool enable_animation = true;

		float make_inert_for_ms = 0.f;

		/* speed at which the response receivers speed multiplier reaches 1.0 */
		float max_speed_for_movement_response = 1.f;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(apply_movement_forces),

				CEREAL_NVP(response_receivers),

				CEREAL_NVP(moving_left),
				CEREAL_NVP(moving_right),
				CEREAL_NVP(moving_forward),
				CEREAL_NVP(moving_backward),
				CEREAL_NVP(walking_enabled),

				CEREAL_NVP(input_acceleration_axes),
				CEREAL_NVP(acceleration_length),

				CEREAL_NVP(applied_force_offset),

				CEREAL_NVP(non_braking_damping),
				CEREAL_NVP(braking_damping),

				CEREAL_NVP(standard_linear_damping),

				CEREAL_NVP(enable_braking_damping),

				CEREAL_NVP(enable_animation),

				CEREAL_NVP(make_inert_for_ms),

				CEREAL_NVP(max_speed_for_movement_response)
			);
		}

		void add_animation_receiver(entity_id e, bool stop_at_zero_movement);
		void reset_movement_flags();
		void set_flags_from_target_direction(vec2 d);
		void set_flags_from_closest_direction(vec2 d);
	};
}