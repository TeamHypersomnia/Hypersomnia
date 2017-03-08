#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(chased_entity));

		f(t.NVP(force_towards_chased_entity));
		f(t.NVP(distance_when_force_easing_starts));
		f(t.NVP(power_of_force_easing_multiplier));

		f(t.NVP(percent_applied_to_chased_entity));

		f(t.NVP(divide_transform_mode));
		f(t.NVP(consider_rotation));

		f(t.NVP(chased_entity_offset));

		f(t.NVP(force_offsets));
	}

}