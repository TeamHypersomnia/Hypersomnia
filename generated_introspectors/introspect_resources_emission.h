#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(spread_degrees));
		f(t.NVP(base_speed));
		f(t.NVP(base_speed_variation));
		f(t.NVP(rotation_speed));
		f(t.NVP(particles_per_sec));
		f(t.NVP(stream_lifetime_ms));
		f(t.NVP(particle_lifetime_ms));
		f(t.NVP(size_multiplier));
		f(t.NVP(acceleration));
		f(t.NVP(angular_offset));
		f(t.NVP(swing_spread));
		f(t.NVP(swings_per_sec));
		f(t.NVP(min_swing_spread));
		f(t.NVP(max_swing_spread));
		f(t.NVP(min_swings_per_sec));
		f(t.NVP(max_swings_per_sec));
		f(t.NVP(swing_spread_change_rate));
		f(t.NVP(swing_speed_change_rate));
		f(t.NVP(fade_when_ms_remaining));
		f(t.NVP(num_of_particles_to_spawn_initially));

		f(t.NVP(randomize_spawn_point_within_circle_of_outer_radius));
		f(t.NVP(randomize_spawn_point_within_circle_of_inner_radius));

		f(t.NVP(starting_spawn_circle_size_multiplier));
		f(t.NVP(ending_spawn_circle_size_multiplier));

		f(t.NVP(starting_homing_force));
		f(t.NVP(ending_homing_force));

		f(t.NVP(homing_target));

		f(t.NVP(initial_rotation_variation));
		f(t.NVP(randomize_acceleration));
		f(t.NVP(should_particles_look_towards_velocity));

		f(t.NVP(particle_templates));
	}

}