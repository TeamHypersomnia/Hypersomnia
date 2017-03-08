#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(shape));
		f(t.NVP(material));

		f(t.NVP(collision_sound_gain_mult));

		f(t.NVP(density));
		f(t.NVP(density_multiplier));
		f(t.NVP(friction));
		f(t.NVP(restitution));
			
		f(t.NVP(filter));
		f(t.NVP(destructible));
		f(t.NVP(sensor));
	}

}