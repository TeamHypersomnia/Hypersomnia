#pragma once
#include "utility/map_wrapper.h"
#include "../messages/particle_burst_message.h"
#include "../components/render_component.h"

#include "../resources/render_info.h"

namespace resources {
	struct particle {
		augmentations::vec2<> pos, vel, acc;
		resources::sprite face;
		float rotation;
		float rotation_speed;
		float linear_damping;
		float angular_damping;
		float lifetime_ms, max_lifetime_ms;
		bool should_disappear;
		particle() : face(nullptr), lifetime_ms(0.f), should_disappear(true), rotation(0.f), rotation_speed(0.f) {}
	};

	struct emission {
		enum type {
			BURST,
			STREAM
		} type;

		float spread_degrees;

		std::pair<float, float>
			velocity,
			angular_velocity,
			particles_per_sec,
			stream_duration_ms,
			particle_lifetime_ms,
			size_multiplier,
			acceleration,
			angular_offset,
			swing_spread,
			swings_per_sec,
			min_swing_spread,
			max_swing_spread,
			min_swings_per_sec,
			max_swings_per_sec,
			swing_spread_change_rate,
			swing_speed_change_rate,
			fade_when_ms_remaining
			;

		std::pair<unsigned, unsigned>
			particles_per_burst;

		float initial_rotation_variation;
		bool randomize_acceleration;

		augmentations::vec2<> offset;

		std::vector<particle> particle_templates;
		components::render particle_render_template;

		void add_particle_template(const particle& p) {
			particle_templates.push_back(p);
		}

		emission() : 
			particles_per_burst(std::make_pair(0, 0)),
			velocity(std::make_pair(0.f, 0.f)),
			angular_velocity(std::make_pair(0.f, 0.f)),
			particles_per_sec(std::make_pair(0.f, 0.f)),
			stream_duration_ms(std::make_pair(0.f, 0.f)),
			particle_lifetime_ms(std::make_pair(0.f, 0.f)),
			size_multiplier(std::make_pair(0.f, 0.f)),
			min_swing_spread(std::make_pair(0.f, 0.f)),
			min_swings_per_sec(std::make_pair(0.f, 0.f)),
			max_swing_spread(std::make_pair(0.f, 0.f)),
			max_swings_per_sec(std::make_pair(0.f, 0.f)),
			swing_spread_change_rate(std::make_pair(0.f, 0.f)),
			swing_speed_change_rate(std::make_pair(0.f, 0.f)),
			fade_when_ms_remaining(std::make_pair(0.f, 0.f)),
			acceleration(std::make_pair(0.f, 0.f)), randomize_acceleration(false), spread_degrees(0.f), initial_rotation_variation(0.f),
			offset(0.f, 0.f), angular_offset(std::make_pair(0.f, 0.f))
		{}
	};

	typedef std::vector<emission> particle_effect;
	typedef augmentations::util::map_wrapper <
		messages::particle_burst_message::burst_type, particle_effect
	> particle_emitter_info;
}