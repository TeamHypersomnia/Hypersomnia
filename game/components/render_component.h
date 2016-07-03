#pragma once
#include "transform_component.h"

#include "math/vec2.h"
#include "game/enums/render_layer.h"
#include "graphics/pixel.h"

namespace components {
	struct render {
		transform previous_transform;
		transform saved_actual_transform;
		
		vec2 interpolation_direction() {
			return saved_actual_transform.pos - previous_transform.pos;
		}

		bool interpolate = true;
		bool snap_interpolation_when_close = true;

		render_layer layer = render_layer::INVALID;

		bool absolute_transform = false;

		bool draw_border = false;
		augs::rgba border_color;

		unsigned last_step_when_visible = 0;
	};
}