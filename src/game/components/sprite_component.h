#pragma once
#include "augs/pad_bytes.h"
#include "augs/drawing/flip.h"
#include "augs/drawing/sprite.h"

#include "game/components/sprite_component_declaration.h"

namespace components {
	struct sprite {
		// GEN INTROSPECTOR struct components::sprite
		flip_flags flip;
		pad_bytes<2> pad;
		// END GEN INTROSPECTOR
	};
};