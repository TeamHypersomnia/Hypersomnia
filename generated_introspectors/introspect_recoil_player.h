#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(offsets));
		f(t.NVP(current_offset));
		f(t.NVP(reversed));
		f(t.NVP(repeat_last_n_offsets));

		f(t.NVP(single_cooldown_duration_ms));
		f(t.NVP(remaining_cooldown_duration));
		f(t.NVP(scale));
	}

}