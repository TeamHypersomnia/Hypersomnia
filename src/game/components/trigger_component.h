#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/padding_byte.h"

namespace components {
	struct trigger {
		// GEN INTROSPECTOR struct components::trigger
		entity_id entity_to_be_notified;
		bool react_to_collision_detectors = false;
		bool react_to_query_detectors = true;
		std::array<padding_byte, 2> pad;
		// END GEN INTROSPECTOR
	};
}
