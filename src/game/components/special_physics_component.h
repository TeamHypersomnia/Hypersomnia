#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "game/transcendental/entity_id.h"
#include "augs/misc/stepped_timing.h"

struct friction_connection {
	// GEN INTROSPECTOR struct friction_connection
	entity_id target;
	unsigned fixtures_connected = 0;
	// END GEN INTROSPECTOR
	friction_connection(entity_id t = entity_id()) : target(t) {}

	bool operator==(entity_id b) const {
		return target == b;
	}

	operator entity_id() const {
		return target;
	}
};

namespace components {
	struct special_physics {
		// GEN INTROSPECTOR struct components::special_physics
		augs::stepped_cooldown dropped_or_created_cooldown;
		entity_id during_cooldown_ignore_collision_with;
		entity_id owner_friction_ground;
		augs::constant_size_vector<friction_connection, OWNER_FRICTION_GROUNDS_COUNT> owner_friction_grounds;
		// END GEN INTROSPECTOR

		//float measured_carried_mass = 0.f;
	};
}