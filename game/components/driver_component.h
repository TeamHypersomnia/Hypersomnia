#pragma once
#include "game/transcendental/entity_id.h"

namespace components {
	struct driver {
		// GEN INTROSPECTOR components::driver
		entity_id owned_vehicle;
		float density_multiplier_while_driving = 1.f/3.f;
		// END GEN INTROSPECTOR
	};
}