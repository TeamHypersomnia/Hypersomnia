#pragma once
#include "entity_system/component.h"
#include "transform_component.h"

#include "math/vec2.h"

namespace components {
	struct render : public augs::component {
		vec2 last_screen_pos;
		bool was_drawn = false;

		transform previous_transform;
		transform saved_actual_transform;
		bool interpolate = true;

		enum mask_type {
			WORLD,
			GUI
		};

		unsigned layer = 0;
		unsigned mask = mask_type::WORLD;

		bool flip_horizontally = false;
		bool flip_vertically = false;
		bool absolute_transform = false;
	};
}