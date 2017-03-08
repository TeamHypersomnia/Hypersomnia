#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(input));

		f(t.NVP(current_displacement));
		f(t.NVP(time_of_last_displacement));
		f(t.NVP(current_displacement_duration_bound_ms));

		f(t.NVP(time_of_birth));
		f(t.NVP(max_lifetime_in_steps));

		f(t.NVP(distribute_within_segment_of_length));
	}

}