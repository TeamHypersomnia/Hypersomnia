#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/math/vec2.h"
#include "game/transcendental/entity_id.h"

#include "padding_byte.h"

namespace components {
	struct movement  {
		struct subscribtion {
			entity_id target;
			bool stop_response_at_zero_speed = false;
			padding_byte pad[3];

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(target),
					CEREAL_NVP(stop_response_at_zero_speed)
				);
			}
		};

		augs::constant_size_vector<subscribtion, MOVEMENT_RESPONSE_RECEIVERS_COUNT> response_receivers;
		
		bool moving_left = false;
		bool moving_right = false;
		bool moving_forward = false;
		bool moving_backward = false;

		bool walking_enabled = false;
		bool enable_braking_damping = false;
		bool enable_animation = true;
		bool apply_movement_forces = true;

		bool sprint_enabled = false;
		padding_byte pad[3];

		vec2 input_acceleration_axes;
		float acceleration_length = -1.f;

		vec2 applied_force_offset;

		float non_braking_damping = 0.f;
		float braking_damping = 0.f;

		float standard_linear_damping = 0.f;

		float make_inert_for_ms = 0.f;

		/* speed at which the response receivers speed multiplier reaches 1.0 */
		float max_speed_for_movement_response = 1.f;
		
		template<class F>
		void for_each_held_id(F f) {
			for (auto& e : response_receivers) {
				f(e.target);
			}
		}

		template<class F>
		void for_each_held_id(F f) const {
			for (const auto& e : response_receivers) {
				f(e.target);
			}
		}

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