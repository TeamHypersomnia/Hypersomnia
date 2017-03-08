#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(center_neon_map));
		f(t.NVP(vertices));
		f(t.NVP(triangulation_indices));

	}

}