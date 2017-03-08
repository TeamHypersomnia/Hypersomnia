#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(orbit_mode));

		f(t.NVP(recoil_entity));

		f(t.NVP(character_entity_to_chase));
		f(t.NVP(base_offset));
		f(t.NVP(bounds_for_base_offset));

		f(t.NVP(visible_world_area));
		f(t.NVP(max_look_expand));

		f(t.NVP(rotation_offset));
		f(t.NVP(size_multiplier));
		f(t.NVP(sensitivity));
	}

}