#pragma once
#include <tuple>
#include "augs/templates/type_mod_templates.h"
#include "augs/templates/transform_types.h"

#include "augs/misc/constant_size_vector.h"

#include "augs/misc/bound.h"
#include "game/enums/particle_layer.h"
#include "view/viewables/particle_types.h"

#include "augs/templates/per_type.h"

struct particle_effect_modifier;

struct particles_emission {
	using bound = augs::bound<float>;
	using random_bound = augs::random_bound<float>;

	template <class T>
	using make_particle_vector = std::vector<T>;

	using particle_definitions_vectors = per_type_container<list_of_particle_types_t<>, make_particle_vector>;

	// GEN INTROSPECTOR struct particles_emission
	bound spread_degrees = bound(0.f, 0.f);
	bound base_speed = bound(0.f, 0.f);
	bound base_speed_variation = bound(0.f, 0.f);
	bound rotation_speed = bound(0.f, 0.f);
	bound particles_per_sec = bound(0.f, 0.f);
	bound stream_lifetime_ms = bound(0.f, 0.f);
	bound particle_lifetime_ms = bound(0.f, 0.f);
	bound size_multiplier = bound(1.f, 1.f);
	bound acceleration = bound(0.f, 0.f);
	bound angular_offset = bound(0.f, 0.f);
	bound swing_spread = bound(0.f, 0.f);
	bound swings_per_sec = bound(0.f, 0.f);
	random_bound swing_spread_bound = { { 0.f, 0.f }, { 0.f, 0.f } };
	random_bound swings_per_sec_bound = { { 0.f, 0.f }, { 0.f, 0.f } };
	bound swing_spread_change_rate = bound(0.f, 0.f);
	bound swing_speed_change_rate = bound(0.f, 0.f);
	bound fade_when_ms_remaining = bound(0.f, 0.f);
	bound num_of_particles_to_spawn_initially = bound(0.f, 0.f);

	bound randomize_spawn_point_within_circle_of_outer_radius = bound(0.f, 0.f);
	bound randomize_spawn_point_within_circle_of_inner_radius = bound(0.f, 0.f);

	bound starting_spawn_circle_size_multiplier = bound(1.f, 1.f);
	bound ending_spawn_circle_size_multiplier = bound(1.f, 1.f);

	bound starting_homing_force = bound(0.f, 0.f);
	bound ending_homing_force = bound(0.f, 0.f);

	float initial_rotation_variation = 0.f;
	bool randomize_acceleration = false;
	bool scale_damping_to_velocity = false;
	bool should_particles_look_towards_velocity = true;
	pad_bytes<1> pad;

	particle_definitions_vectors particle_definitions;
	particle_layer target_layer = particle_layer::ILLUMINATING_PARTICLES;
	// END GEN INTROSPECTOR

	template <class T>
	auto& get_definitions() {
		return particle_definitions.get_for<T>();
	}

	template <class T>
	const auto& get_definitions() const {
		return particle_definitions.get_for<T>();
	}

	template <class T>
	void add_particle_definition(const T& t) {
		get_definitions<T>().push_back(t);
	}

	template <class T>
	bool has() const {
		return get_definitions<T>().size() > 0;
	}
};

using emission_vector = augs::constant_size_vector<particles_emission, MAX_PARTICLE_EMISSIONS>;

struct particle_effect {
	// GEN INTROSPECTOR struct particle_effect
	emission_vector emissions;
	std::string name;
	// END GEN INTROSPECTOR

	const auto& get_name() const {
		return name;
	}
};