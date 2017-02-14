#pragma once
#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

#include "augs/misc/minmax.h"

namespace resources {
	struct particle {
		vec2 pos;
		vec2 vel;
		vec2 acc;
		components::sprite face;
		float rotation = 0.f;
		float rotation_speed = 0.f;
		float linear_damping = 0.f;
		float angular_damping = 0.f;
		float lifetime_ms = 0.f;
		float max_lifetime_ms = 0.f;
		float shrink_when_ms_remaining = 0.f;
		float unshrinking_time_ms = 0.f;

		bool ignore_rotation = false;
		int alpha_levels = -1;

		void integrate(const float dt);

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(pos),
				CEREAL_NVP(vel),
				CEREAL_NVP(acc),
				CEREAL_NVP(face),
				CEREAL_NVP(rotation),
				CEREAL_NVP(rotation_speed),
				CEREAL_NVP(linear_damping),
				CEREAL_NVP(angular_damping),
				CEREAL_NVP(lifetime_ms),
				CEREAL_NVP(max_lifetime_ms),
				CEREAL_NVP(should_disappear),
				CEREAL_NVP(ignore_rotation),
				CEREAL_NVP(alpha_levels)
			);
		}
	};

	struct particle_effect_modifier {
		rgba colorize;
		float scale_amounts = 1.f;
		float scale_lifetimes = 1.f;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(colorize),
				CEREAL_NVP(scale_amounts),
				CEREAL_NVP(scale_lifetimes)
			);
		}
	};

	struct emission {
		augs::minmax<float>
			spread_degrees = std::make_pair(0.f, 0.f),
			base_velocity = std::make_pair(0.f, 0.f),
			base_velocity_variation = std::make_pair(0.f, 0.f),
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
		
		float initial_rotation_variation = 0.f;
		bool randomize_acceleration = false;
		bool should_particles_look_towards_velocity = true;

		vec2 offset;

		std::vector<particle> particle_templates;
		components::render particle_render_template;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(type),
				CEREAL_NVP(spread_degrees),
				CEREAL_NVP(velocity),
				CEREAL_NVP(angular_velocity),
				CEREAL_NVP(particles_per_sec),
				CEREAL_NVP(stream_duration_ms),
				CEREAL_NVP(particle_lifetime_ms),
				CEREAL_NVP(size_multiplier),
				CEREAL_NVP(acceleration),
				CEREAL_NVP(angular_offset),
				CEREAL_NVP(swing_spread),
				CEREAL_NVP(swings_per_sec),
				CEREAL_NVP(min_swing_spread),
				CEREAL_NVP(max_swing_spread),
				CEREAL_NVP(min_swings_per_sec),
				CEREAL_NVP(max_swings_per_sec),
				CEREAL_NVP(swing_spread_change_rate),
				CEREAL_NVP(swing_speed_change_rate),
				CEREAL_NVP(fade_when_ms_remaining),
				CEREAL_NVP(num_of_particles_to_spawn_initially),

				CEREAL_NVP(initial_rotation_variation),
				CEREAL_NVP(randomize_acceleration),

				CEREAL_NVP(offset),

				CEREAL_NVP(particle_templates),
				CEREAL_NVP(particle_render_template)
			);
		}

		void apply_modifier(particle_effect_modifier m);
	};

	typedef std::vector<emission> particle_effect;
}