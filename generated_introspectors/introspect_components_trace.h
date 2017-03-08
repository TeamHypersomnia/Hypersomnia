#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(max_multiplier_x));
		f(t.NVP(max_multiplier_y));

		f(t.NVP(chosen_multiplier));

		f(t.NVP(lengthening_duration_ms));
		f(t.NVP(chosen_lengthening_duration_ms));
		f(t.NVP(lengthening_time_passed_ms));

		f(t.NVP(is_it_finishing_trace));
	}

}