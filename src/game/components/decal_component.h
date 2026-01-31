#pragma once
#include "augs/math/declare_math.h"
#include "game/cosmos/entity_id.h"

namespace invariants {
	struct decal {
		// GEN INTROSPECTOR struct invariants::decal
		real32 lifetime_secs = 60.f;
		real32 start_shrinking_when_remaining_ms = 5000.f;
		bool is_blood_decal = false;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct decal {
		// GEN INTROSPECTOR struct components::decal
		real32 last_size_mult = 1.f;
		// END GEN INTROSPECTOR
	};
}
