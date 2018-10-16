#pragma once
#include <tuple>
#include "augs/templates/type_mod_templates.h"
#include "augs/templates/transform_types.h"

#include "augs/misc/minmax.h"
#include "game/enums/particle_layer.h"
#include "view/viewables/particle_types.h"

#include "augs/templates/per_type.h"

struct particle_effect_modifier;

struct particles_emission {
	using minmax = augs::minmax<float>;
	using random_bound = augs::random_bound<float>;

	template <class T>
	using make_particle_vector = std::vector<T>;

	using particle_definitions_vectors = per_type_container<list_of_particle_types_t<>, make_particle_vector>;

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
	random_bound swing_spread_bound = { { 0.f, 0.f }, { 0.f, 0.f } };
	random_bound swings_per_sec_bound = { { 0.f, 0.f }, { 0.f, 0.f } };
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

struct particle_effect {
	// GEN INTROSPECTOR struct particle_effect
	std::vector<particles_emission> emissions;
	std::string name;
	// END GEN INTROSPECTOR

	const auto& get_name() const {
		return name;
	}
};