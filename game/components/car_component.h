#pragma once
#include "game/entity_id.h"
#include "math/vec2.h"

namespace components {
	struct car {
		entity_id current_driver;

		entity_id left_wheel_trigger;
		entity_id right_wheel_trigger;

		bool accelerating = false;
		bool deccelerating = false;
		bool turning_right = false;
		bool turning_left = false;
		bool hand_brake = false;
		
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

		void reset_movement_flags();
	};
}