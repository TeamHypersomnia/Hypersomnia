#pragma once
#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

namespace resources {
	struct particle {
		vec2 pos, vel, acc;
		components::sprite face;
		float rotation = 0.f;
		float rotation_speed = 0.f;
		float linear_damping = 0.f;
		float angular_damping = 0.f;
		float lifetime_ms = 0.f, max_lifetime_ms = 0.f;
		bool should_disappear = true;
		bool ignore_rotation = false;
		int alpha_levels = -1;
	};

	struct particle_effect_modifier {
		augs::rgba colorize;
		float scale_amounts = 1.f;
		float scale_lifetimes = 1.f;
	};

	struct emission {
		enum type {
			BURST,
			STREAM
		} type;

		std::pair<float, float>
			spread_degrees = std::make_pair(0.f, 0.f),
			velocity = std::make_pair(0.f, 0.f),
			angular_velocity = std::make_pair(0.f, 0.f),
			particles_per_sec = std::make_pair(0.f, 0.f),
			stream_duration_ms = std::make_pair(0.f, 0.f),
			particle_lifetime_ms = std::make_pair(0.f, 0.f),
			size_multiplier = std::make_pair(0.f, 0.f),
			acceleration = std::make_pair(0.f, 0.f),
			angular_offset = std::make_pair(0.f, 0.f),
			swing_spread = std::make_pair(0.f, 0.f),
			swings_per_sec = std::make_pair(0.f, 0.f),
			min_swing_spread = std::make_pair(0.f, 0.f),
			max_swing_spread = std::make_pair(0.f, 0.f),
			min_swings_per_sec = std::make_pair(0.f, 0.f),
			max_swings_per_sec = std::make_pair(0.f, 0.f),
			swing_spread_change_rate = std::make_pair(0.f, 0.f),
			swing_speed_change_rate = std::make_pair(0.f, 0.f),
			fade_when_ms_remaining = std::make_pair(0.f, 0.f),
			num_of_particles_to_spawn_initially = std::make_pair(0.f, 0.f)
			;

		std::pair<unsigned, unsigned> particles_per_burst = std::make_pair(0u, 0u);

		float initial_rotation_variation = 0.f;
		bool randomize_acceleration = false;

		vec2 offset;

		std::vector<particle> particle_templates;
		components::render particle_render_template;

		void apply_modifier(particle_effect_modifier m);
	};

	typedef std::vector<emission> particle_effect;
}