#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(screen_space_transform));
		f(t.NVP(draw_border));
		f(t.NVP(layer));

		f(t.NVP(border_color));

		f(t.NVP(last_step_when_visible));
	}

}