#pragma once
#include "augs/pad_bytes.h"
#include "game/cosmos/entity_id.h"

namespace components {
	struct driver {
		// GEN INTROSPECTOR struct components::driver
		signi_entity_id owned_vehicle;
		float density_multiplier_while_driving = 0.02f;
		// END GEN INTROSPECTOR
	};
}