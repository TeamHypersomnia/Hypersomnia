#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(min_value));
		f(t.NVP(max_value));
		f(t.NVP(change_speed));
	}

}