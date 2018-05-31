#pragma once
#include "augs/templates/value_with_flag.h"
#include "game/detail/footstep_effect.h"

namespace invariants {
	struct ground {
		// GEN INTROSPECTOR struct invariants::ground
		augs::value_with_flag<footstep_effect_input> footstep_effect;
		// END GEN INTROSPECTOR
	};
}
