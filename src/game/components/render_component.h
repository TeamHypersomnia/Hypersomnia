#pragma once
#include "transform_component.h"

#include "augs/math/vec2.h"
#include "game/enums/render_layer.h"
#include "augs/graphics/rgba.h"

#include "augs/pad_bytes.h"

namespace components {
	struct render {
		// GEN INTROSPECTOR struct components::render
		render_layer layer = render_layer::INVALID;
		// END GEN INTROSPECTOR
	};
}