#pragma once
#include "game/container_sizes.h"

#include "augs/math/vec2.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/pad_bytes.h"

namespace invariants {
	struct movement;
}

struct movement_animation_state {
	// GEN INTROSPECTOR struct movement_animation_state
	unsigned index = 0;
	unsigned base_frames_n = 5;
	bool flip = false;
	bool backward = false;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR

	unsigned get_multi_way_index(const unsigned animation_frames_n) const {
		if (animation_frames_n == base_frames_n) {
			return index;
		}

		if (animation_frames_n == base_frames_n * 2) {
			return get_two_ways_index();
		}

		if (animation_frames_n == base_frames_n * 4) {
			return get_four_ways_index();
		}

		return 0;
	}

	unsigned get_two_ways_index() const {
		if (backward) {
			return 2 * base_frames_n - index - 1;
		}

		return index;
	}

	unsigned get_four_ways_index() const {
		const auto two_ways = get_two_ways_index();

		if (flip) {
			return 4 * base_frames_n - two_ways - 1;
		}

		return two_ways;
	}
};

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
		float animation_amount = 0.f;

		movement_animation_state four_ways_animation;
		// END GEN INTROSPECTOR

		bool any_moving_requested() const {
			return 
				moving_left
				|| moving_right
				|| moving_forward
				|| moving_backward
			;
		}

		void reset_movement_flags();
		vec2 get_force_requested_by_input(const vec2& axes) const;
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

		float max_speed_for_animation = 700.f;
		unsigned animation_frame_ms = 30;
		unsigned animation_frame_num = 5;
		// END GEN INTROSPECTOR
	};
}