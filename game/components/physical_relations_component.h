#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"
#include "game/transcendental/entity_id.h"

namespace components {
	struct physical_relations {
		// GEN INTROSPECTOR components::physical_relations
		entity_id owner_body;
		augs::constant_size_vector<entity_id, FIXTURE_ENTITIES_COUNT> fixture_entities;
		// END GEN INTROSPECTOR
	};
}
