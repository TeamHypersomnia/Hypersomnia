#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(colliders));
		f(t.NVP(offsets_for_created_shapes));

		f(t.NVP(activated));
		f(t.NVP(is_friction_ground));
		f(t.NVP(disable_standard_collision_resolution));
		f(t.NVP(can_driver_shoot_through));
	}

}