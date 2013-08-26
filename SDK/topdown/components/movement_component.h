#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"

namespace components {
	struct movement : public augmentations::entity_system::component {
		enum {
			FORWARD, BACKWARD, LEFT, RIGHT
		};

		struct subscribtion {
			augmentations::entity_system::entity_ptr target;
			bool stop_at_zero_movement;
			subscribtion(augmentations::entity_system::entity* target, bool stop_at_zero_movement = true) :
				target(target), stop_at_zero_movement(stop_at_zero_movement) {}
		};

		std::vector<subscribtion> animation_receivers;

		bool current_directions[4];

		augmentations::vec2<> acceleration;
		float max_speed;

		movement(augmentations::vec2<> acceleration, float max_speed) : acceleration(acceleration), max_speed(max_speed) {
			current_directions[0] = current_directions[1] = current_directions[2] = current_directions[3] = false;
		}
	};
}