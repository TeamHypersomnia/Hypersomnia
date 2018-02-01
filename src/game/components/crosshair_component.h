#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/sprite_component.h"

namespace components {
	struct crosshair {
		enum orbit_type {
			NONE,
			ANGLED,
			LOOK
		};

		// GEN INTROSPECTOR struct components::crosshair
		orbit_type orbit_mode = LOOK;

		child_entity_id recoil_entity;

		vec2 base_offset;
		vec2 base_offset_bound;

		float rotation_offset = 0.f;
		vec2 sensitivity = vec2(1.0f, 1.0f);
		// END GEN INTROSPECTOR

		vec2 get_bounds_in_this_look() const;
	};
}

namespace invariants {
	struct crosshair {
		using implied_component = components::crosshair;

		// GEN INTROSPECTOR struct invariants::crosshair
		invariants::sprite appearance;
		// END GEN INTROSPECTOR
	};
}