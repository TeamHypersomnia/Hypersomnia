#pragma once
#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

#include "augs/misc/minmax.h"
#include "augs/zeroed_pod.h"

#include "augs/templates/type_mod_templates.h"
#include "game/detail/particle_types_declaration.h"

#include "game/transcendental/entity_id.h"

struct general_particle;

struct particle_effect_modifier {
	// GEN INTROSPECTOR struct particle_effect_modifier
	rgba colorize;
	float scale_amounts = 1.f;
	float scale_lifetimes = 1.f;
	entity_id homing_target;
	// END GEN INTROSPECTOR
};	

struct particles_emission {
	typedef augs::minmax<float> minmax;

	// GEN INTROSPECTOR struct particles_emission
	minmax spread_degrees = minmax(0.f, 0.f);
	minmax base_speed = minmax(0.f, 0.f);
	minmax base_speed_variation = minmax(0.f, 0.f);
	minmax rotation_speed = minmax(0.f, 0.f);
	minmax particles_per_sec = minmax(0.f, 0.f);
	minmax stream_lifetime_ms = minmax(0.f, 0.f);
	minmax particle_lifetime_ms = minmax(0.f, 0.f);
	minmax size_multiplier = minmax(1.f, 1.f);
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

	minmax randomize_spawn_point_within_circle_of_outer_radius = minmax(0.f, 0.f);
	minmax randomize_spawn_point_within_circle_of_inner_radius = minmax(0.f, 0.f);

	minmax starting_spawn_circle_size_multiplier = minmax(1.f, 1.f);
	minmax ending_spawn_circle_size_multiplier = minmax(1.f, 1.f);

	minmax starting_homing_force = minmax(0.f, 0.f);
	minmax ending_homing_force = minmax(0.f, 0.f);

	entity_id homing_target;

	float initial_rotation_variation = 0.f;
	bool randomize_acceleration = false;
	bool should_particles_look_towards_velocity = true;

	tuple_of_particle_types_t<make_vector> particle_templates;
	// END GEN INTROSPECTOR

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

	void apply_modifier(const particle_effect_modifier m);
};

namespace resources {
	typedef std::vector<particles_emission> particle_effect;
}