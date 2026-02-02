#pragma once
#include "game/detail/view_input/money_type.h"

namespace invariants {
	struct destructible {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::destructible
		real32 max_health = 100.0f;
		real32 make_dynamic_below_area = 0.6f; // Fraction of original area below which static bodies become dynamic. 0 = never become dynamic.
		real32 disable_below_area = 64.0f * 64.0f; // No longer split below this area (in pixels²)
		money_type money_spawned_min = 0;
		money_type money_spawned_max = 0;
		// END GEN INTROSPECTOR
	};
}
