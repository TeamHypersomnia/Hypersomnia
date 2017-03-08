#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(repetitions));
		f(t.NVP(gain));
		f(t.NVP(pitch));
		f(t.NVP(max_distance));
		f(t.NVP(reference_distance));
		f(t.NVP(fade_on_exit));
	}

}