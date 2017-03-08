#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(current_animation));

		f(t.NVP(priority));
		f(t.NVP(frame_num));
		f(t.NVP(player_position_ms));
		f(t.NVP(speed_factor));

		f(t.NVP(state));
		f(t.NVP(paused_state));
	}

}