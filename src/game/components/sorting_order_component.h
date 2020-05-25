#pragma once
#include "transform_component.h"

#include "augs/math/vec2.h"
#include "game/enums/render_layer.h"
#include "augs/graphics/rgba.h"

#include "augs/pad_bytes.h"
#include "game/components/sorting_order_type.h"

namespace invariants {
	struct sorting_order {
		// GEN INTROSPECTOR struct invariants::sorting_order
		sorting_order_type order = 0;
		// END GEN INTROSPECTOR
	};
}