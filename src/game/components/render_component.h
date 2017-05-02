#pragma once
#include "transform_component.h"

#include "augs/math/vec2.h"
#include "game/enums/render_layer.h"
#include "augs/graphics/pixel.h"

#include "augs/padding_byte.h"

namespace components {
	struct render {
		// GEN INTROSPECTOR struct components::render
		bool draw_border = false;
		render_layer layer = render_layer::INVALID;
		std::array<padding_byte, 2> pad;

		rgba border_color;

		unsigned last_step_when_visible = 0;
		// END GEN INTROSPECTOR
	};
}