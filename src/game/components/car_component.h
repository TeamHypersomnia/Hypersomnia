#pragma once
#include "game/cosmos/entity_id.h"
#include "augs/math/vec2.h"
#include "augs/pad_bytes.h"
#include "augs/misc/timing/stepped_timing.h"

struct car_engine_entities {
	// GEN INTROSPECTOR struct car_engine_entities
	child_entity_id physical;
	child_entity_id particles;
	// END GEN INTROSPECTOR
};

namespace components {
	struct car {
		// GEN INTROSPECTOR struct components::car
		signi_entity_id current_driver;

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
		
		real32 braking_damping = -1.f;
		real32 braking_angular_damping = -1.f;

		vec2 input_acceleration = vec2(3500, 3500);

		real32 acceleration_length = -1.f;

		real32 maximum_speed_with_static_air_resistance = 500.f;
		real32 maximum_speed_with_static_damping = 150.f;
		real32 static_air_resistance = 0.009f;
		real32 dynamic_air_resistance = 0.00006f;
		real32 static_damping = 4.6f;
		real32 dynamic_damping = 0.4f;

		real32 maximum_lateral_cancellation_impulse = 20.f;
		real32 lateral_impulse_multiplier = 0.75f;

		real32 angular_damping = 2.f;
		real32 angular_damping_while_hand_braking = 0.8f;

		real32 minimum_speed_for_maneuverability_decrease = 500.f;
		real32 maneuverability_decrease_multiplier = (1.f/4000.f) * 0.06f;

		real32 angular_air_resistance = 0.f;
		real32 angular_air_resistance_while_hand_braking = 0.f;

		real32 speed_for_pitch_unit = 3000.f;

		vec2 wheel_offset = vec2(200, 125);

		augs::stepped_timestamp last_turned_on;
		augs::stepped_timestamp last_turned_off;
		// END GEN INTROSPECTOR

		void reset_movement_flags();
	};
}