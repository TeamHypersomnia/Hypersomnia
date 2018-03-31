#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/math/simple_physics.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/sprite_component.h"

enum class crosshair_orbit_type {
	// GEN INTROSPECTOR enum class crosshair_orbit_type
	NONE,
	ANGLED,
	LOOK
	// END GEN INTROSPECTOR
};

namespace components {
	struct crosshair {
		// GEN INTROSPECTOR struct components::crosshair
		crosshair_orbit_type orbit_mode = crosshair_orbit_type::LOOK;

		simple_body recoil;

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
		// GEN INTROSPECTOR struct invariants::crosshair
		invariants::sprite appearance;
		damping_input recoil_damping;

		impulse_mults recoil_impulse_mult = { 10000.f, 500.f };
		// END GEN INTROSPECTOR
	};
}