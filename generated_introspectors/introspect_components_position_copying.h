#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(target));

		f(t.NVP(offset));
		f(t.NVP(rotation_orbit_offset));
		
		f(t.NVP(reference_position));
		f(t.NVP(target_reference_position));
		
		f(t.NVP(scrolling_speed));

		f(t.NVP(rotation_offset));
		f(t.NVP(rotation_multiplier));

		f(t.NVP(position_copying_mode));
		f(t.NVP(position_copying_rotation));
		f(t.NVP(track_origin));
		f(t.NVP(target_newly_set));
		f(t.NVP(previous));
	}

}