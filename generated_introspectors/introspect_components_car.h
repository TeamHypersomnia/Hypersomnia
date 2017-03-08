#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(current_driver));

		f(t.NVP(interior));

		f(t.NVP(left_wheel_trigger));
		f(t.NVP(right_wheel_trigger));

		f(t.NVP(acceleration_engine));
		f(t.NVP(deceleration_engine));

		f(t.NVP(left_engine));
		f(t.NVP(right_engine));

		f(t.NVP(engine_sound));

		f(t.NVP(accelerating));
		f(t.NVP(decelerating));
		f(t.NVP(turning_right));
		f(t.NVP(turning_left));

		f(t.NVP(hand_brake));
		
		f(t.NVP(braking_damping));
		f(t.NVP(braking_angular_damping));

		f(t.NVP(input_acceleration));

		f(t.NVP(acceleration_length));

		f(t.NVP(maximum_speed_with_static_air_resistance));
		f(t.NVP(maximum_speed_with_static_damping));
		f(t.NVP(static_air_resistance));
		f(t.NVP(dynamic_air_resistance));
		f(t.NVP(static_damping));
		f(t.NVP(dynamic_damping));

		f(t.NVP(maximum_lateral_cancellation_impulse));
		f(t.NVP(lateral_impulse_multiplier));

		f(t.NVP(angular_damping));
		f(t.NVP(angular_damping_while_hand_braking));

		f(t.NVP(minimum_speed_for_maneuverability_decrease));
		f(t.NVP(maneuverability_decrease_multiplier));

		f(t.NVP(angular_air_resistance));
		f(t.NVP(angular_air_resistance_while_hand_braking));

		f(t.NVP(speed_for_pitch_unit));

		f(t.NVP(wheel_offset));

		f(t.NVP(last_turned_on));
		f(t.NVP(last_turned_off));
	}

}