#pragma once
#include <optional>
#include "3rdparty/Box2D/Dynamics/b2Filter.h"

#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "game/cosmos/entity_id.h"

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
		bool bullets_fly_through = false;

		b2Filter filter;

		assets::physical_material_id material;

		real32 collision_sound_gain_mult = 1.f;

		real32 density = 1.f;
		real32 friction = 0.f;
		real32 restitution = 0.f;

		real32 max_ricochet_angle = 10.f;
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
