#pragma once
#include "augs/pad_bytes.h"
#include "augs/drawing/flip.h"
#include "augs/drawing/sprite.h"

#include "game/components/sprite_component_declaration.h"

namespace components {
	struct sprite {
		// GEN INTROSPECTOR struct components::sprite
		bool disable_neon_map = false;
		pad_bytes<3> pad;
		real32 effect_offset_secs = 0.f;
		rgba colorize = white;
		rgba colorize_neon = white;
		// END GEN INTROSPECTOR
	};

	struct overridden_geo {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::overridden_geo
		augs::maybe<vec2i> size;
		flip_flags flip;
		pad_bytes<2> pad;
		// END GEN INTROSPECTOR
	};
};