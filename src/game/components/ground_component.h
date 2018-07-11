#pragma once
#include "augs/templates/maybe.h"
#include "game/detail/footstep_effect.h"

namespace invariants {
	struct ground {
		// GEN INTROSPECTOR struct invariants::ground
		augs::maybe<footstep_effect_input> footstep_effect;
		// END GEN INTROSPECTOR
	};
}
