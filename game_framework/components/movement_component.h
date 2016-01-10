#pragma once
#include <Box2D/Dynamics/b2Fixture.h>
#include "entity_system/component.h"
#include "../messages/animation_response_message.h"
#include "math/vec2.h"

namespace components {
	struct movement : public augs::component {
		struct subscribtion {
			augs::entity_id target;
			bool stop_at_zero_movement;
			subscribtion(augs::entity_id target, bool stop_at_zero_movement = true) :
				target(target), stop_at_zero_movement(stop_at_zero_movement) {}
		};

		void add_animation_receiver(augs::entity_id e, bool stop_at_zero_movement) {
			animation_receivers.push_back(subscribtion(e, stop_at_zero_movement));
		}

		/* entities whom the animation_message will be sent to, including information about subject entity's movement speed */
		std::vector<subscribtion> animation_receivers;

		/* levers controlled by intent messages to induce movement */
		int moving_left = 0, moving_right = 0, moving_forward = 0, moving_backward = 0;
		
		/* default acceleration vector used for movement requested by input */
		vec2 input_acceleration;
	
		/* used to truncate acceleration vector if for example both up and right buttons are pressed */
		float max_accel_len = -1.f;

		/* if this is non-zero, the movement vector will be mapped to XY axis based on this vector and not on the inputs */
		vec2 requested_movement;
			
		/* force is applied to the body's center by default, you can offset this position by changing this value */
		vec2 force_offset;
		
		/* two-dimensional damping value (though you're most likely interested in only X axis) 
		applied when the character is not moving in the side-scrolling environment */
		vec2 inverse_thrust_brake;

		/* a physically realistic alternative to max_speed variable, the bigger the value is, the lesser the maximum speed */
		float air_resistance = 0.f;

		float max_speed = -1.f;

		/* used for all environments; apply general damping value when no movement is requested */
		float braking_damping = -1.f;

		/* speed at which the receivers' animation speed multiplier reaches 1.0 */
		float max_speed_animation = 1.f;

		/* angular offset for all forces
		used only for side-scrolling environments */
		float axis_rotation_degrees = 0.f;

		bool sidescroller_setup = false;

		/* if this is non-zero, a ray of length of this variable is cast under the player to determine the ground angle and rotate the movement forces
		to be applied for slopes accordingly
		used only for side-scrolling environments
		*/
		float thrust_parallel_to_ground_length = 0.f;

		/* filter for the aforementioned ray cast */
		b2Filter ground_filter;
	};
}