#pragma once
#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "game/cosmos/entity_id.h"
#include "game/assets/ids/asset_ids.h"
#include "game/components/transform_component.h"

class physics_world_cache;
struct motor_joint_cache;
struct b2Fixture_index_in_component;

namespace components {
	struct motor_joint {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::motor_joint
		std::array<signi_entity_id, 2> target_bodies;

		bool activated = true;
		bool collide_connected = false;
		pad_bytes<2> pad;

		vec2 linear_offset;
		float angular_offset = 0.f;
		float max_force = 1.f;
		float max_torque = 1.f;
		float correction_factor = 0.3f;
		// END GEN INTROSPECTOR
	};
}
