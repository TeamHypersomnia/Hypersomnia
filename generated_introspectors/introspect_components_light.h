#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(color));

		f(t.NVP(constant));
		f(t.NVP(linear));
		f(t.NVP(quadratic));
		f(t.NVP(max_distance));
		
		f(t.NVP(wall_constant));
		f(t.NVP(wall_linear));
		f(t.NVP(wall_quadratic));
		f(t.NVP(wall_max_distance));

		f(t.NVP(position_variations));
	}

}