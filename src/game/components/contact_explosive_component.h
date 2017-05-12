#pragma once

#include "game/detail/explosions.h"
#include "game/transcendental/entity_id.h"

namespace components {
	struct contact_explosive {
		// GEN INTROSPECTOR struct components::contact_explosive
		entity_id ignore_collision_with;
		standard_explosion_input explosion_defenition;
		// END GEN INTROSPECTOR
	};
}
