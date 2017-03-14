#pragma once
#include "game/transcendental/entity_id.h"
#include "game/enums/grenade_type.h"

namespace components {
	struct grenade {
		// GEN INTROSPECTOR struct components::grenade
		child_entity_id spoon;
		entity_id released_spoon;
		grenade_type type = grenade_type::INVALID;
		// END GEN INTROSPECTOR
	};
}
