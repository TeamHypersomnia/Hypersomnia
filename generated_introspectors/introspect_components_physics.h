#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(fixed_rotation));
		f(t.NVP(bullet));
		f(t.NVP(angled_damping));
		f(t.NVP(activated));

		f(t.NVP(body_type));

		f(t.NVP(angular_damping));
		f(t.NVP(linear_damping));
		f(t.NVP(linear_damping_vec));
		f(t.NVP(gravity_scale));

		f(t.NVP(transform));
		f(t.NVP(sweep));

		f(t.NVP(velocity));
		f(t.NVP(angular_velocity));
	}

}