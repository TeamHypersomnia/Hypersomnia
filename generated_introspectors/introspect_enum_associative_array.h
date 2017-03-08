#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F, class Enum, class T>
	void introspect(
		maybe_const_ref_t<C, enum_associative_array<Enum, T>> t,
		F f
	) {
		f(t.NVP(is_set));
		f(t.NVP(raw));
	}

}