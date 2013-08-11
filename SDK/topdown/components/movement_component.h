#pragma once
#include "entity_system/component.h"
#include "math/vec2d.h"

namespace components {
	struct movement : public augmentations::entity_system::component {
		enum {
			FORWARD, BACKWARD, LEFT, RIGHT
		};

		bool current_directions[4];

		augmentations::vec2<float> acceleration;
		float max_speed;

		movement(augmentations::vec2<float> acceleration, float max_speed) : acceleration(acceleration), max_speed(max_speed) {
			current_directions[0] = current_directions[1] = current_directions[2] = current_directions[3] = false;
		}
	};
}