#pragma once
#include <optional>
#include <Box2D/Dynamics/b2Fixture.h>

#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "game/transcendental/entity_id.h"

#include "game/enums/colliders_offset_type.h"
#include "game/assets/ids/asset_ids.h"
#include "game/detail/physics/colliders_connection.h"

#include "game/components/transform_component.h"

namespace components {
	struct specific_colliders_connection {
		// GEN INTROSPECTOR struct components::specific_colliders_connection
		colliders_connection connection;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct fixtures {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::fixtures
		bool friction_ground = false;
		bool disable_standard_collision_resolution = false;
		bool driver_shoot_through = false;
		bool destructible = false;

		bool sensor = false;

		pad_bytes<1> pad;
		b2Filter filter;

		assets::physical_material_id material;

		float collision_sound_gain_mult = 1.f;

		float density = 1.f;
		float friction = 0.f;
		float restitution = 0.f;
		// END GEN INTROSPECTOR

		/*
			Interface for when we want to calculate these depending on a cached value
		*/

		bool is_friction_ground() const {
			return friction_ground;
		}	

		bool is_destructible() const {
			return destructible;
		}

		bool can_driver_shoot_through() const {
			return driver_shoot_through;
		}

		bool standard_collision_resolution_disabled() const {
			return disable_standard_collision_resolution;
		}
	};
}
