#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"
#include "padding_byte.h"
#include "augs/misc/stepped_timing.h"

namespace components {
	struct car {
		entity_id current_driver;

		child_entity_id interior;

		child_entity_id left_wheel_trigger;
		child_entity_id right_wheel_trigger;

		struct engine_entities {
			child_entity_id physical;
			child_entity_id particles;
		};

		engine_entities acceleration_engine[2];
		engine_entities deceleration_engine[2];

		engine_entities left_engine;
		engine_entities right_engine;

		child_entity_id engine_sound;

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

		float speed_for_pitch_unit = 3000.f;

		vec2 wheel_offset = vec2(200, 125);

		augs::stepped_timestamp last_turned_on;
		augs::stepped_timestamp last_turned_off;

		template<class F>
		void for_each_held_id(F f) {
			f(current_driver);

			f(left_wheel_trigger);
			f(right_wheel_trigger);

			for (auto& e : acceleration_engine_particles) {
				f(e);
			}

			for (auto& e : deceleration_engine) {
				f(e);
			}

			f(left_engine_particles);
			f(right_engine_particles);

			f(engine_sound);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(current_driver);

			f(left_wheel_trigger);
			f(right_wheel_trigger);

			for (const auto& e : acceleration_engine_particles) {
				f(e);
			}

			for (const auto& e : deceleration_engine) {
				f(e);
			}

			f(left_engine_particles);
			f(right_engine_particles);

			f(engine_sound);
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

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, image::paint_circle_midpoint_command> t,
		F f
	) {


	}
}