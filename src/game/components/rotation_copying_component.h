#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/math/vec2.h"
#include "augs/padding_byte.h"

namespace components {
	struct rotation_copying {
		enum class look_type {
			POSITION,
			VELOCITY,
			ROTATION
		};

		enum class easing_type {
			NONE,
			LINEAR,
			EXPONENTIAL
		};
		
		// GEN INTROSPECTOR struct components::rotation_copying
		entity_id target;
		entity_id stashed_target;

		easing_type easing_mode = easing_type::NONE;

		bool colinearize_item_in_hand = false;
		bool update_value = true;
		pad_bytes<2> pad;
		
		float smoothing_average_factor = 0.5f;
		float averages_per_sec = 20.0f;
		
		vec2 last_rotation_interpolant;

		look_type look_mode = look_type::POSITION;
		look_type stashed_look_mode = look_type::POSITION;
		// END GEN INTROSPECTOR

		void stash() {
			stashed_look_mode = look_mode;
			stashed_target = target;
		}

		void unstash() {
			look_mode = stashed_look_mode;
			target = stashed_target;
		}
	};
}