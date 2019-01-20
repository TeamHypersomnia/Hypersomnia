#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/timing/delta.h"

namespace augs {
	struct speed_vars {
		// GEN INTROSPECTOR struct augs::speed_vars
		unsigned tickrate = 60u;
		real32 logic_speed_mult = 1.f;
		// END GEN INTROSPECTOR

		auto calc_inv_tickrate() const {
			return 1.0 / tickrate;
		}

		auto calc_ticking_delta() const {
			return augs::delta::steps_per_second(tickrate);
		}

		auto calc_fixed_delta() const {
			if (logic_speed_mult == 1.f) {
				return calc_ticking_delta();
			}

			return augs::delta::from_milliseconds(logic_speed_mult * real32(1000.f) / tickrate);
		}
	};
}
