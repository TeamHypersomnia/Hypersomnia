#pragma once

#include "game/entity_id.h"
#include "math/vec2.h"

namespace components {
	struct rotation_copying  {
		enum look_type {
			POSITION,
			VELOCITY,
			ACCELEARATION
		};

		enum rotation_copying_easing {
			NONE,
			LINEAR,
			EXPONENTIAL
		};
		
		int easing_mode = NONE;
		bool colinearize_item_in_hand = false;
		
		/* for exponential smoothing */
		double smoothing_average_factor = 0.5;
		double averages_per_sec = 20.0;
		
		/* for linear smoothing */
		vec2 last_rotation_interpolant;

		unsigned look_mode = look_type::POSITION;

		entity_id target;

		float last_value = 0.0f;
		bool update_value = true;

		bool use_physical_motor = false;
	};
}