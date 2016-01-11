#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_id.h"
#include "math/vec2.h"

namespace components {
	struct lookat : public augs::component {
		enum look_type {
			POSITION,
			VELOCITY,
			ACCELEARATION
		};

		enum lookat_easing {
			NONE,
			LINEAR,
			EXPONENTIAL
		};
		
		int easing_mode = NONE;
		
		/* for exponential smoothing */
		double smoothing_average_factor = 0.5;
		double averages_per_sec = 20.0;
		
		/* for linear smoothing */
		vec2 last_rotation_interpolant;

		unsigned look_mode = look_type::POSITION;

		augs::entity_id target;

		float last_value = 0.0f;
		bool update_value = true;

		bool use_physical_motor = false;
	};
}