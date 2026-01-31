#pragma once
#include "augs/math/declare_math.h"
#include "game/cosmos/entity_id.h"

namespace invariants {
	struct decal {
		// GEN INTROSPECTOR struct invariants::decal
		bool is_blood_decal = false;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct decal {
		// GEN INTROSPECTOR struct components::decal
		real32 last_size_mult = 1.f;
		bool marked_for_deletion = false;
		// END GEN INTROSPECTOR
	};
}
