#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(target));
		f(t.NVP(stashed_target));

		f(t.NVP(easing_mode));

		f(t.NVP(colinearize_item_in_hand));
		f(t.NVP(update_value));
		
		f(t.NVP(smoothing_average_factor));
		f(t.NVP(averages_per_sec));
		
		f(t.NVP(last_rotation_interpolant));

		f(t.NVP(look_mode));
		f(t.NVP(stashed_look_mode));
	}

}