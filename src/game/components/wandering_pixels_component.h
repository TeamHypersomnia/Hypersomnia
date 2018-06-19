#pragma once
#include "game/components/sprite_component.h"
#include "augs/misc/constant_size_vector.h"

namespace components {
	struct wandering_pixels {
		// GEN INTROSPECTOR struct components::wandering_pixels
		vec2 size;
		rgba colorize = white;
		unsigned particles_count = 0u;
		// END GEN INTROSPECTOR
	};
}

using wandering_pixels_frames = augs::constant_size_vector<invariants::sprite, 10>;

namespace invariants {
	struct wandering_pixels {
		// GEN INTROSPECTOR struct invariants::wandering_pixels
		assets::plain_animation_id animation_id;
		// END GEN INTROSPECTOR
	};
}