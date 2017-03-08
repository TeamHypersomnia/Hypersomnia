#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(response_receivers));
		
		f(t.NVP(moving_left));
		f(t.NVP(moving_right));
		f(t.NVP(moving_forward));
		f(t.NVP(moving_backward));

		f(t.NVP(walking_enabled));
		f(t.NVP(enable_braking_damping));
		f(t.NVP(enable_animation));
		f(t.NVP(sprint_enabled));

		f(t.NVP(input_acceleration_axes));
		f(t.NVP(acceleration_length));

		f(t.NVP(applied_force_offset));

		f(t.NVP(non_braking_damping));
		f(t.NVP(braking_damping));

		f(t.NVP(standard_linear_damping));

		f(t.NVP(make_inert_for_ms));
		f(t.NVP(max_speed_for_movement_response));
	}

}