#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"
#include "augs/padding_byte.h"
#include "augs/misc/stepped_timing.h"

struct car_engine_entities {
	// GEN INTROSPECTOR struct car_engine_entities
	child_entity_id physical;
	child_entity_id particles;
	// END GEN INTROSPECTOR
};

namespace components {
	struct car {
		// GEN INTROSPECTOR struct components::car
		entity_id current_driver;

		child_entity_id interior;

		child_entity_id left_wheel_trigger;
		child_entity_id right_wheel_trigger;

		std::array<car_engine_entities, 2> acceleration_engine;
		std::array<car_engine_entities, 2> deceleration_engine;

		car_engine_entities left_engine;
		car_engine_entities right_engine;

		child_entity_id engine_sound;

		bool accelerating = false;
		bool decelerating = false;
		bool turning_right = false;
		bool turning_left = false;

		bool hand_brake = false;
		pad_bytes<3> pad;
		
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

		float speed_for_pitch_unit = 3000.f;

		vec2 wheel_offset = vec2(200, 125);

		augs::stepped_timestamp last_turned_on;
		augs::stepped_timestamp last_turned_off;
		// END GEN INTROSPECTOR

		void reset_movement_flags();
	};
}