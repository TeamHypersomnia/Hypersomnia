#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(tex));
		f(t.NVP(color));
		f(t.NVP(size));
		f(t.NVP(size_multiplier));
		f(t.NVP(center_offset));
		f(t.NVP(rotation_offset));

		f(t.NVP(flip_horizontally));
		f(t.NVP(flip_vertically));
		
		f(t.NVP(effect));
		f(t.NVP(has_neon_map));

		f(t.NVP(max_specular_blinks));
	}

}