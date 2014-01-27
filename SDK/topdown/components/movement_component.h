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

		void add_animation_receiver(augmentations::entity_system::entity* e, bool stop_at_zero_movement) {
			animation_receivers.push_back(subscribtion(e, stop_at_zero_movement));
		}

		std::vector<subscribtion> animation_receivers;

		bool moving_left, moving_right, moving_forward, moving_backward;
		augmentations::vec2<> input_acceleration, force_offset;
		
		augmentations::vec2<> inverse_thrust_brake;

		float air_resistance;

		float max_speed;
		float braking_damping;

		float max_speed_animation;

		float axis_rotation_degrees;
		float axis_speed_constraint;

		float thrust_parallel_to_ground_length;
		b2Filter ground_filter;

		movement(augmentations::vec2<> acceleration = augmentations::vec2<>(), float air_resistance = 0.f) 
			: input_acceleration(input_acceleration), air_resistance(air_resistance), braking_damping(-1.f), max_speed(-1.f), max_speed_animation(0.f), 
			axis_rotation_degrees(0.f), axis_speed_constraint(-1.f), thrust_parallel_to_ground_length(0.f) {
			moving_left = moving_right = moving_forward = moving_backward = false;
		}
	};
}