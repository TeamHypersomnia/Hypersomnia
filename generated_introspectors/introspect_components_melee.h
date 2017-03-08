#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(primary_move_flag));
		f(t.NVP(secondary_move_flag));
		f(t.NVP(tertiary_move_flag));

		f(t.NVP(current_state));
	}

}