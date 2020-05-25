#pragma once
#include "transform_component.h"
#include "augs/math/vec2.h"
#include "game/enums/render_layer.h"
#include "augs/graphics/rgba.h"
#include "augs/pad_bytes.h"
#include "augs/misc/enum/enum_boolset.h"
#include "game/detail/special_render_function.h"

namespace invariants {
	struct render {
		// GEN INTROSPECTOR struct invariants::render
		render_layer layer = render_layer::GROUND;
		augs::enum_boolset<special_render_function> special_functions;
		// END GEN INTROSPECTOR
	};
}