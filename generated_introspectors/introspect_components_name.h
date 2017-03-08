#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(id));

		f(t.NVP(custom_nickname));
		f(t.NVP(nickname));
	}

}