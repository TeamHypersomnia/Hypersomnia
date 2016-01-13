#pragma once
#include "entity_system/entity.h"
#include "math/vec2.h"

namespace components {
	struct car {
		augs::entity_id current_driver;

		augs::entity_id left_wheel_trigger;
		augs::entity_id right_wheel_trigger;

		bool accelerating = false;
		bool deccelerating = false;
		bool turning_right = false;
		bool turning_left = false;

		void reset_movement_flags() {
			accelerating = deccelerating = turning_left = turning_right = false;
		}
		
		float braking_damping = -1.f;

		vec2 input_acceleration = vec2(1500, 1500);
	};
}