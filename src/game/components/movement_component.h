#pragma once
#include "game/container_sizes.h"

#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"

#include "augs/pad_bytes.h"
#include "augs/misc/timing/stepped_timing.h"

#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "augs/misc/bound.h"

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

struct movement_flags {
	// GEN INTROSPECTOR struct movement_flags
	bool left = false;
	bool right = false;
	bool forward = false;
	bool backward = false;

	bool walking = false;
	pad_bytes<1> pad;
	bool sprinting = false;
	bool dashing = false;
	// END GEN INTROSPECTOR

	bool any_moving_requested() const {
		return left || right || forward || backward;
	}

	vec2 get_force_requested_by_input(const vec2& axes) const;
	void set_flags_from_target_direction(vec2 d);
	void set_from_closest_direction(vec2 d);
};

namespace components {
	struct movement {
		// GEN INTROSPECTOR struct components::movement
		movement_flags flags;

		bool frozen = false;
		bool was_sprint_effective = false;
		bool was_walk_effective = false;
		bool forward_moves_towards_crosshair = false;

		real32 surface_slowdown_ms = 0.f;

		real32 dash_cooldown_ms = 0.f;
		real32 const_inertia_ms = 0.f;
		real32 linear_inertia_ms = 0.f;
		real32 portal_inertia_ms = 0.f;

		real32 animation_amount = 0.f;

		movement_animation_state four_ways_animation;

		uint8_t blood_step_counter = 0;
		uint8_t _total_blood_steps_cache = 0;
		pad_bytes<2> pad;
		augs::stepped_timestamp _oldest_footstep_stamp;
		// END GEN INTROSPECTOR

		auto get_max_inertia() const {
			return std::max(std::max(const_inertia_ms, linear_inertia_ms), portal_inertia_ms);
		}

		void reset_movement_flags();
	};
}

namespace invariants {
	struct movement {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::movement
		vec2 applied_force_offset;
		vec2 input_acceleration_axes;

		real32 acceleration_length = -1.f;
		real32 standard_linear_damping = 0.f;
		real32 non_braking_damping = 0.f;
		real32 braking_damping = 0.f;

		real32 max_linear_inertia_when_movement_possible = 150.f;
		real32 const_inertia_mult = 0.1f;

		real32 freeze_legs_when_inertia_exceeds = 50.f;

		real32 max_speed_for_animation = 700.f;
		unsigned animation_frame_ms = 30;
		unsigned animation_frame_num = 5;

		real32 dash_cooldown_ms = 1000.f;
		real32 dash_cooldown_mult_after_transfer = 0.2f;

		real32 dash_impulse = 1163.f;
		real32 dash_inert_ms = 400.f;

		particle_effect_input dash_particles;
		sound_effect_input dash_sound;

		augs::bound<real32> dash_effect_bound = augs::bound<real32> { 0.88f, 1.0f };

		real32 surface_slowdown_max_ms = 200.f;
		real32 surface_slowdown_unit_ms = 100.f;
		real32 surface_drag_mult = 0.5f;
		// END GEN INTROSPECTOR
	};
}