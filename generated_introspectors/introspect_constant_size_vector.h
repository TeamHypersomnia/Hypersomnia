#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F, class T, int const_count>
	void introspect(
		maybe_const_ref_t<C, constant_size_vector<T, const_count>> t,
		F f
	) {
		f(t.NVP(count));
		f(t.NVP(raw));
	}

}