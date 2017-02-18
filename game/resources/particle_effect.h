#pragma once
#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

#include "augs/misc/minmax.h"
#include "augs/zeroed_pod.h"

#include "augs/templates/type_mod_templates.h"
#include "game/detail/particle_types_declaration.h"

struct general_particle;

namespace resources {
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
		typedef augs::minmax<float> minmax;

		minmax spread_degrees = minmax(0.f, 0.f);
		minmax base_speed = minmax(0.f, 0.f);
		minmax base_speed_variation = minmax(0.f, 0.f);
		minmax angular_velocity = minmax(0.f, 0.f);
		minmax particles_per_sec = minmax(0.f, 0.f);
		minmax stream_duration_ms = minmax(0.f, 0.f);
		minmax particle_lifetime_ms = minmax(0.f, 0.f);
		minmax size_multiplier = minmax(0.f, 0.f);
		minmax acceleration = minmax(0.f, 0.f);
		minmax angular_offset = minmax(0.f, 0.f);
		minmax swing_spread = minmax(0.f, 0.f);
		minmax swings_per_sec = minmax(0.f, 0.f);
		minmax min_swing_spread = minmax(0.f, 0.f);
		minmax max_swing_spread = minmax(0.f, 0.f);
		minmax min_swings_per_sec = minmax(0.f, 0.f);
		minmax max_swings_per_sec = minmax(0.f, 0.f);
		minmax swing_spread_change_rate = minmax(0.f, 0.f);
		minmax swing_speed_change_rate = minmax(0.f, 0.f);
		minmax fade_when_ms_remaining = minmax(0.f, 0.f);
		minmax num_of_particles_to_spawn_initially = minmax(0.f, 0.f);
		
		float initial_rotation_variation = 0.f;
		bool randomize_acceleration = false;
		bool should_particles_look_towards_velocity = true;

		vec2 offset;

		put_all_particle_types_into_t<make_vector> particle_templates;

		template <class T>
		auto& get_templates() {
			return std::get<std::vector<T>>(particle_templates);
		}

		template <class T>
		const auto& get_templates() const {
			return std::get<std::vector<T>>(particle_templates);
		}

		template <class T>
		void add_particle_template(const T& t) {
			get_templates<T>().push_back(t);
		}

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

		void apply_modifier(const particle_effect_modifier m);
	};

	typedef std::vector<emission> particle_effect;
}