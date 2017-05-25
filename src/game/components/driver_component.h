#pragma once
#include "augs/padding_byte.h"
#include "game/transcendental/entity_id.h"

namespace components {
	struct driver {
		// GEN INTROSPECTOR struct components::driver
		entity_id owned_vehicle;
		float density_multiplier_while_driving = 1.f/3.f;
		bool take_hold_of_wheel_when_touched = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}