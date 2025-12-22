#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/math/simple_physics.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/components/sprite_component.h"

enum class crosshair_orbit_type {
	// GEN INTROSPECTOR enum class crosshair_orbit_type
	NONE,
	ANGLED,
	LOOK,
	COUNT
	// END GEN INTROSPECTOR
};

namespace components {
	struct crosshair {
		// GEN INTROSPECTOR struct components::crosshair
		crosshair_orbit_type orbit_mode = crosshair_orbit_type::LOOK;
		simple_body recoil;
		vec2 base_offset;
		bool zoom_out_mode = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
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