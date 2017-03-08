#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(dropped_collision_cooldown));
		f(t.NVP(owner_friction_ground));
		f(t.NVP(owner_friction_grounds));
	}

}