#pragma once
#include "game/transcendental/entity_id.h"
#include "game/enums/child_entity_name.h"

namespace components {
	struct logical_child {
		static constexpr bool is_always_present = true;

		// GEN INTROSPECTOR struct components::logical_child
		entity_id parent;
		// END GEN INTROSPECTOR
	};
}
