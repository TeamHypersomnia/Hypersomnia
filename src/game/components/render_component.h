#pragma once
#include <cstdint>
#include "transform_component.h"
#include "augs/math/vec2.h"
#include "game/enums/render_layer.h"
#include "augs/graphics/rgba.h"
#include "augs/pad_bytes.h"
#include "augs/misc/enum/enum_boolset.h"
#include "game/detail/special_render_function.h"

namespace invariants {
	/*
		shadow_height is in pixels of height, which cosmos_light_settings::shadow_step turns into the length of sun shadows.
		reaches_ceiling makes point lights never shine over the obstacle, whatever its shadow height.
		Both are read only for entities with physical bodies.
	*/

	struct render {
		// GEN INTROSPECTOR struct invariants::render
		render_layer layer = render_layer::GROUND;
		augs::enum_boolset<special_render_function> special_functions;
		uint8_t shadow_height = 32;
		bool reaches_ceiling = false;
		pad_bytes<2> pad;
		// END GEN INTROSPECTOR
	};
}