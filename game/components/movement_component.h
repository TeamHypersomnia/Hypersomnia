#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/math/vec2.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/padding_byte.h"

struct movement_subscribtion {
	// GEN INTROSPECTOR struct movement_subscribtion
	entity_id target;
	bool stop_response_at_zero_speed = false;
	std::array<padding_byte, 3> pad;
	// END GEN INTROSPECTOR
};

namespace components {
	struct movement  {
		// GEN INTROSPECTOR struct components::movement
		augs::constant_size_vector<movement_subscribtion, MOVEMENT_RESPONSE_RECEIVERS_COUNT> response_receivers;
		
		bool moving_left = false;
		bool moving_right = false;
		bool moving_forward = false;
		bool moving_backward = false;

		bool walking_enabled = false;
		bool enable_braking_damping = false;
		bool enable_animation = true;
		bool sprint_enabled = false;

		vec2 input_acceleration_axes;
		float acceleration_length = -1.f;

		vec2 applied_force_offset;

		float non_braking_damping = 0.f;
		float braking_damping = 0.f;

		float standard_linear_damping = 0.f;

		float make_inert_for_ms = 0.f;
		float max_speed_for_movement_event = 1.f;
		// END GEN INTROSPECTOR

		void add_animation_receiver(entity_id e, bool stop_at_zero_movement);
		void reset_movement_flags();
		vec2 get_force_requested_by_input() const;
		void set_flags_from_target_direction(vec2 d);
		void set_flags_from_closest_direction(vec2 d);
	};
}