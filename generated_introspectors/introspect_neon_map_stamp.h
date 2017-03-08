#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(standard_deviation));
		f(t.NVP(radius_towards_x_axis));
		f(t.NVP(radius_towards_y_axis));
		f(t.NVP(amplification));
		f(t.NVP(alpha_multiplier));
		f(t.NVP(last_write_time_of_source));

		f(t.NVP(light_colors));
	}

}