#pragma once
#include "augs/pad_bytes.h"
#include "augs/drawing/flip.h"
#include "augs/drawing/sprite.h"

#include "game/components/sprite_component_declaration.h"

namespace components {
	struct sprite {
		// GEN INTROSPECTOR struct components::sprite
		flip_flags flip;
		bool disable_neon_map = false;
		pad_bytes<1> pad;
		float effect_offset_secs = 0.f;
		rgba colorize = white;
		// END GEN INTROSPECTOR
	};
};