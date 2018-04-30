#pragma once
#include "game/container_sizes.h"

#include "augs/math/vec2.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/pad_bytes.h"

namespace invariants {
	struct movement;
}

namespace components {
	struct movement {
		// GEN INTROSPECTOR struct components::movement
		bool moving_left = false;
		bool moving_right = false;
		bool moving_forward = false;
		bool moving_backward = false;

		bool walking_enabled = false;
		bool sprint_enabled = false;
		bool was_sprint_effective = false;
		pad_bytes<1> pad;

		float make_inert_for_ms = 0.f;
		// END GEN INTROSPECTOR

		void reset_movement_flags();
		vec2 get_force_requested_by_input(const invariants::movement&) const;
		void set_flags_from_target_direction(vec2 d);
		void set_flags_from_closest_direction(vec2 d);
	};
}

namespace invariants {
	struct movement {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::movement
		vec2 applied_force_offset;
		vec2 input_acceleration_axes;

		float acceleration_length = -1.f;
		float standard_linear_damping = 0.f;
		float non_braking_damping = 0.f;
		float braking_damping = 0.f;
		// END GEN INTROSPECTOR
	};
}