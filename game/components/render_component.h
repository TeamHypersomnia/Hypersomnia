#pragma once
#include "transform_component.h"

#include "math/vec2.h"
#include "game/enums/render_layer.h"
#include "graphics/pixel.h"

namespace components {
	struct render {
		vec2 last_screen_pos;
		bool was_drawn = false;

		transform previous_transform;
		transform saved_actual_transform;
		
		vec2 interpolation_direction() {
			return saved_actual_transform.pos - previous_transform.pos;
		}

		bool interpolate = true;
		bool snap_interpolation_when_close = true;

		enum mask_type {
			WORLD,
		};

		render_layer layer = render_layer::INVALID;
		unsigned mask = mask_type::WORLD;

		bool absolute_transform = false;

		bool draw_border = false;
		augs::rgba border_color;

		float partial_overlay_height_ratio = 0.f;
		augs::rgba partial_overlay_color;

		int layer_priority = 0;
		unsigned last_step_when_visible = 0;
		int last_visibility_index = -1;
	};
}