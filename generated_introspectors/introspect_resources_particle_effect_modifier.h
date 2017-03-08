#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(colorize));
		f(t.NVP(scale_amounts));
		f(t.NVP(scale_lifetimes));
		f(t.NVP(homing_target));
	}

}