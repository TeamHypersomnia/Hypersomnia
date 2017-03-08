#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(path));
		f(t.NVP(characters));
		
		f(t.NVP(pt));
	}

}