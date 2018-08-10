#pragma once
#include "game/cosmos/entity_id.h"
#include "game/enums/adverse_element_type.h"
#include "game/assets/ids/asset_ids.h"
#include "game/detail/explosions.h"
#include "game/detail/explosive/cascade_explosion_input.h"

namespace invariants {
	struct explosive {
		// GEN INTROSPECTOR struct invariants::explosive
		standard_explosion_input explosion;
		std::array<cascade_explosion_input, 3> cascade;
		// END GEN INTROSPECTOR
	};
}
