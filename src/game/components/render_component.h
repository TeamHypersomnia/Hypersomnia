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
		shadow_height is in levels of cosmos_light_settings::shadow_step.
		It is read only for entities with physical bodies.
	*/

	struct render {
		// GEN INTROSPECTOR struct invariants::render
		render_layer layer = render_layer::GROUND;
		augs::enum_boolset<special_render_function> special_functions;
		uint8_t shadow_height = 32;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}