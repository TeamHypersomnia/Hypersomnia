#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"

namespace components {
	struct movement : public augmentations::entity_system::component {
		struct subscribtion {
			augmentations::entity_system::entity_ptr target;
			bool stop_at_zero_movement;
			subscribtion(augmentations::entity_system::entity* target, bool stop_at_zero_movement = true) :
				target(target), stop_at_zero_movement(stop_at_zero_movement) {}
		};

		void add_animation_receiver(augmentations::entity_system::entity_ptr e, bool stop_at_zero_movement) {
			animation_receivers.push_back(subscribtion(e, stop_at_zero_movement));
		}

		std::vector<subscribtion> animation_receivers;

		bool moving_left, moving_right, moving_forward, moving_backward;
		augmentations::vec2<> input_acceleration;
		
		float max_speed;

		movement(augmentations::vec2<> acceleration = augmentations::vec2<>(), float max_speed = 0.f) : input_acceleration(input_acceleration), max_speed(max_speed) {
			moving_left = moving_right = moving_forward = moving_backward = false;
		}
	};
}