#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"
#include "padding_byte.h"

namespace components {
	struct car {
		entity_id current_driver;

		entity_id left_wheel_trigger;
		entity_id right_wheel_trigger;

		entity_id acceleration_engine;
		entity_id deceleration_engine;
		entity_id left_engine;
		entity_id right_engine;

		bool accelerating = false;
		bool decelerating = false;
		bool turning_right = false;
		bool turning_left = false;

		bool hand_brake = false;
		padding_byte pad[3];
		
		float braking_damping = -1.f;
		float braking_angular_damping = -1.f;

		vec2 input_acceleration = vec2(3500, 3500);

		float acceleration_length = -1.f;

		float maximum_speed_with_static_air_resistance = 500.f;
		float maximum_speed_with_static_damping = 150.f;
		float static_air_resistance = 0.009f;
		float dynamic_air_resistance = 0.00006f;
		float static_damping = 4.6f;
		float dynamic_damping = 0.4f;

		float maximum_lateral_cancellation_impulse = 20.f;
		float lateral_impulse_multiplier = 0.75f;

		float angular_damping = 2.f;
		float angular_damping_while_hand_braking = 0.8f;

		float minimum_speed_for_maneuverability_decrease = 500.f;
		float maneuverability_decrease_multiplier = (1.f/4000.f) * 0.06f;

		float angular_air_resistance = 0.f;
		float angular_air_resistance_while_hand_braking = 0.f;

		vec2 wheel_offset = vec2(200, 125);

		template<class F>
		void for_each_held_id(F f) {
			f(current_driver);

			f(acceleration_engine);
			f(deceleration_engine);
			f(left_engine);
			f(right_engine);

			f(left_wheel_trigger);
			f(right_wheel_trigger);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(current_driver);

			f(acceleration_engine);
			f(deceleration_engine);
			f(left_engine);
			f(right_engine);

			f(left_wheel_trigger);
			f(right_wheel_trigger);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(current_driver),

				CEREAL_NVP(left_wheel_trigger),
				CEREAL_NVP(right_wheel_trigger),

				CEREAL_NVP(accelerating),
				CEREAL_NVP(decelerating),
				CEREAL_NVP(turning_right),
				CEREAL_NVP(turning_left),
				CEREAL_NVP(hand_brake),

				CEREAL_NVP(braking_damping),
				CEREAL_NVP(braking_angular_damping),

				CEREAL_NVP(input_acceleration),

				CEREAL_NVP(acceleration_length),

				CEREAL_NVP(maximum_speed_with_static_air_resistance),
				CEREAL_NVP(maximum_speed_with_static_damping),
				CEREAL_NVP(static_air_resistance),
				CEREAL_NVP(dynamic_air_resistance),
				CEREAL_NVP(static_damping),
				CEREAL_NVP(dynamic_damping),

				CEREAL_NVP(maximum_lateral_cancellation_impulse),
				CEREAL_NVP(lateral_impulse_multiplier),

				CEREAL_NVP(angular_damping),
				CEREAL_NVP(angular_damping_while_hand_braking),

				CEREAL_NVP(minimum_speed_for_maneuverability_decrease),
				CEREAL_NVP(maneuverability_decrease_multiplier),

				CEREAL_NVP(angular_air_resistance),
				CEREAL_NVP(angular_air_resistance_while_hand_braking),

				CEREAL_NVP(wheel_offset)
				);
		}

		void reset_movement_flags();
	};
}