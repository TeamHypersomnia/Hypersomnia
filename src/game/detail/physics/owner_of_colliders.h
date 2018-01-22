#pragma once
#include "game/transcendental/entity_id.h"
#include "game/components/transform_component.h"

struct owner_of_colliders {
	// GEN INTROSPECTOR struct owner_of_colliders
	entity_id owner;
	components::transform shape_offset;
	// END GEN INTROSPECTOR

	bool operator==(const owner_of_colliders& b) const {
		return owner == b.owner && shape_offset == b.shape_offset;
	}

	bool operator!=(const owner_of_colliders& b) const {
		return !operator==(b);
	}
};
