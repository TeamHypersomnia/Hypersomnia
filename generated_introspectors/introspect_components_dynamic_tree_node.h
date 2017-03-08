#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(always_visible));
		f(t.NVP(activated));
		f(t.NVP(type));


		f(t.NVP(aabb));
	}

}