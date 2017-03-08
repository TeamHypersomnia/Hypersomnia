#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(entity_to_be_notified));
		f(t.NVP(react_to_collision_detectors));
		f(t.NVP(react_to_query_detectors));
	}

}