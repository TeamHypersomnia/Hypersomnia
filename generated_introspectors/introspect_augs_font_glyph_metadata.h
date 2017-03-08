#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(adv));
		f(t.NVP(bear_x));
		f(t.NVP(bear_y));
		f(t.NVP(index));
		f(t.NVP(unicode));

		f(t.NVP(size));

		f(t.NVP(kerning));
	}

}