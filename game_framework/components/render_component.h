#pragma once
#include "transform_component.h"

#include "math/vec2.h"
#include "../globals/layers.h"

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

		enum mask_type {
			WORLD,
		};

		render_layer layer = render_layer::BOTTOM;
		unsigned mask = mask_type::WORLD;

		bool absolute_transform = false;

		int rendering_proxy;

		int layer_priority = 0;
		unsigned last_step_when_visible = 0;
	};
}