#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(effect));
		f(t.NVP(delete_entity_after_effect_lifetime));

		f(t.NVP(modifier));

		f(t.NVP(displace_source_position_within_radius));
		f(t.NVP(single_displacement_duration_ms));
	}

}