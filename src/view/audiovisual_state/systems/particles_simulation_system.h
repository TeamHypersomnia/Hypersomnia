#pragma once
#include <unordered_map>

#include "augs/misc/timing/delta.h"
#include "augs/misc/minmax.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/enums/render_layer.h"
#include "game/components/particles_existence_component.h"

#include "view/viewables/all_viewables_declarations.h"
#include "view/viewables/particle_types_declaration.h"
#include "view/viewables/particle_effect.h"

class interpolation_system;

class particles_simulation_system {
public:
	struct emission_instance {
		using minmax = augs::minmax<float>;

		bool enable_streaming = false;
		pad_bytes<2> pad;

		float angular_offset = 0.f;

		float stream_lifetime_ms = 0.0;
		float stream_max_lifetime_ms = 0.0;
		float stream_particles_to_spawn = 0.f;

		float spread = 0.f;
		float particles_per_sec = 0.f;

		float swing_spread = 0.f;
		float swings_per_sec = 0.f;
		minmax swing_spread_bound = {};
		minmax swings_per_sec_bound = {};
		float swing_spread_change = 0.f;
		float swing_speed_change = 0.f;

		float randomize_spawn_point_within_circle_of_outer_radius = 0.f;
		float randomize_spawn_point_within_circle_of_inner_radius = 0.f;

		float starting_homing_force = 0.f;
		float ending_homing_force = 0.f;

		float starting_spawn_circle_size_multiplier = 0.f;
		float ending_spawn_circle_size_multiplier = 0.f;

		augs::minmax<float> particle_speed;

		float fade_when_ms_remaining = 0.f;

		particles_emission source_emission;

		void stop_streaming() {
			enable_streaming = false;
		}
	};

	struct cache {
		components::particles_existence recorded_existence;

		std::vector<emission_instance> emission_instances;
		bool constructed = false;
	};

	per_render_layer_t<std::vector<general_particle>> general_particles;
	per_render_layer_t<std::vector<animated_particle>> animated_particles;
	per_render_layer_t<std::unordered_map<entity_id, std::vector<homing_animated_particle>>> homing_animated_particles;

	std::unordered_map<entity_id, cache> per_entity_cache;

	void reserve_caches_for_entities(const size_t) {}
	void clear();
	void clear_dead_entities(const cosmos&);

	cache& get_cache(const const_entity_handle);

	void add_particle(const render_layer, const general_particle&);
	void add_particle(const render_layer, const animated_particle&);
	void add_particle(const render_layer, const entity_id, const homing_animated_particle&);

	template <class particle_type, class rng_type>
	auto spawn_particle(
		rng_type& rng,
		const float angular_offset,
		const augs::minmax<float> speed,
		const vec2 position,
		const float basic_velocity_degrees,
		const float spread,
		const particles_emission& emission
	) {
		const auto& templates = emission.get_definitions<particle_type>();
		auto new_particle = templates[rng.randval(0u, static_cast<unsigned>(templates.size()) - 1)];
		
		const auto velocity_degrees = basic_velocity_degrees + angular_offset + rng.randval(spread);
		const auto new_velocity = vec2::from_degrees(velocity_degrees) * rng.randval(speed);
		
		new_particle.set_velocity(new_velocity);
		new_particle.set_position(position);
		new_particle.multiply_size(rng.randval(emission.size_multiplier));
		
		if (emission.should_particles_look_towards_velocity) {
			new_particle.set_rotation(rng.randval(emission.initial_rotation_variation) + velocity_degrees);
		}
		else {
			new_particle.set_rotation(rng.randval(emission.initial_rotation_variation));
		}

		new_particle.set_rotation_speed(rng.randval(emission.rotation_speed));
		new_particle.set_max_lifetime_ms(rng.randval(emission.particle_lifetime_ms));

		if (emission.randomize_acceleration) {
			new_particle.set_acceleration(
				vec2::from_degrees(
					rng.randval(spread) + basic_velocity_degrees
				) * rng.randval(emission.acceleration)
			);
		}

		return new_particle;
	}

	void advance_visible_streams_and_all_particles(
		camera_cone,
		const vec2 screen_size,
		const cosmos&,
		const particle_effects_map&,
		const augs::delta dt,
		const interpolation_system&
	);

	template <class M>
	void draw_particles_as_sprites(
		const M& manager,
		const definitions::sprite::drawing_input basic_input,
		const render_layer layer
	) const {
		for (const auto& it : general_particles[layer]) {
			it.draw_as_sprite(manager, basic_input);
		}

		for (const auto& it : animated_particles[layer]) {
			it.draw_as_sprite(manager, basic_input);
		}

		for (const auto& cluster : homing_animated_particles[layer]) {
			for (const auto& it : cluster.second) {
				it.draw_as_sprite(manager, basic_input);
			}
		}
	}
};