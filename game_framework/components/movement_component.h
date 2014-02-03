#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"

namespace components {
	struct movement : public augs::entity_system::component {
		struct subscribtion {
			augs::entity_system::entity_ptr target;
			bool stop_at_zero_movement;
			subscribtion(augs::entity_system::entity* target, bool stop_at_zero_movement = true) :
				target(target), stop_at_zero_movement(stop_at_zero_movement) {}
		};

		void add_animation_receiver(augs::entity_system::entity* e, bool stop_at_zero_movement) {
			animation_receivers.push_back(subscribtion(e, stop_at_zero_movement));
		}

		/* entities whom the animation_message will be sent to, including information about subject entity's movement speed */
		std::vector<subscribtion> animation_receivers;

		/* levers controlled by intent messages to induce movement */
		bool moving_left, moving_right, moving_forward, moving_backward;
		
		/* default acceleration vector used for movement requested by input */
		augs::vec2<> input_acceleration;

		/* if this is non-zero, the movement vector will be mapped to XY axis based on this vector and not on the inputs */
		augs::vec2<> requested_movement;
			
		/* force is applied to the body's center by default, you can offset this position by changing this value */
		augs::vec2<> force_offset;
		
		/* two-dimensional damping value (though you're most likely interested in only X axis) 
		applied when the character is not moving in the side-scrolling environment */
		augs::vec2<> inverse_thrust_brake;

		/* a physically realistic alternative to max_speed variable, the bigger the value is, the lesser the maximum speed */
		float air_resistance;

		float max_speed;

		/* used for all environments; apply general damping value when no movement is requested */
		float braking_damping;

		/* speed at which the receivers' animation speed multiplier reaches 1.0 */
		float max_speed_animation;

		/* angular offset for all forces
		used only for side-scrolling environments */
		float axis_rotation_degrees;

		/* if this is non-zero, a ray of length of this variable is cast under the player to determine the ground angle and rotate the movement forces
		to be applied for slopes accordingly
		used only for side-scrolling environments
		*/
		float thrust_parallel_to_ground_length;

		/* filter for the aforementioned ray cast */
		b2Filter ground_filter;

		movement(augs::vec2<> acceleration = augs::vec2<>(), float air_resistance = 0.f) 
			: input_acceleration(input_acceleration), air_resistance(air_resistance), braking_damping(-1.f), max_speed(-1.f), max_speed_animation(0.f), 
			axis_rotation_degrees(0.f), thrust_parallel_to_ground_length(0.f) {
			moving_left = moving_right = moving_forward = moving_backward = false;
		}
	};
}