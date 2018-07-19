#pragma once
#include <unordered_map>
#include "augs/misc/simple_pair.h"

#include "augs/misc/timing/delta.h"
#include "augs/misc/minmax.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/enums/particle_layer.h"
#include "game/detail/view_input/particle_effect_input.h"

#include "view/audiovisual_state/systems/audiovisual_cache_common.h"
#include "view/viewables/all_viewables_declarations.h"
#include "view/viewables/particle_effect.h"

class interpolation_system;
struct randomization;

class particles_simulation_system {
public:
	struct emission_instance {
		using minmax = augs::minmax<float>;

		float angular_offset = 0.f;

		float stream_lifetime_ms = 0.0;

	private:
		float stream_max_lifetime_ms = 0.f;
	public:

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

		minmax particle_speed;

		float fade_when_ms_remaining = 0.f;

		particles_emission source_emission;

		bool is_over() const;
		float calc_alivity_mult() const;
		float advance_lifetime_get_dt(const augs::delta& dt, bool stream_infinitely);

		void init_bounds(
			const particles_emission& emission,
			randomization& rng
		);

		emission_instance() = default;
		emission_instance(
			const particles_emission& emission,
			randomization& rng
		);
	};

	using emission_instances = std::vector<emission_instance>;

	static bool are_over(const emission_instances& instances) {
		for (const auto& instance : instances) {
			if (!instance.is_over()) {
				return false;
			}
		}

		return true;
	}

	struct effect_not_found {};

	struct basic_cache {
		std::vector<emission_instance> emission_instances;
		packaged_particle_effect original;

		basic_cache() = default;

		basic_cache(
			const packaged_particle_effect& original,
			const particle_effects_map& manager,
			randomization& rng
		);

		bool is_over() const {
			return are_over(emission_instances);
		}
	};

	struct orbital_cache : basic_cache {
		absolute_or_local chasing;

		orbital_cache() = default;

		template <class... Args>
		orbital_cache(
			const absolute_or_local& chasing, Args&&... args
		) : 
			basic_cache(std::forward<Args>(args)...), 
			chasing(chasing) 
		{}
	};

	struct faf_cache : basic_cache {
		transformr transform;

		faf_cache() = default;

		template <class... Args>
		faf_cache(
			const transformr& transform, Args&&... args
		) : 
			basic_cache(std::forward<Args>(args)...), 
			transform(transform) 
		{}
	};

	/* Particle vectors */
	per_particle_layer_t<std::vector<general_particle>> general_particles;
	per_particle_layer_t<std::vector<animated_particle>> animated_particles;
	per_particle_layer_t<std::unordered_map<entity_id, std::vector<homing_animated_particle>>> homing_animated_particles;

	/* Current streams vectors */
	std::vector<orbital_cache> orbital_emissions;
	std::vector<faf_cache> fire_and_forget_emissions;

	audiovisual_cache_map<orbital_cache> firearm_engine_caches;
	audiovisual_cache_map<orbital_cache> continuous_particles_caches;

	void clear();
	void clear_dead_entities(const cosmos&);

	void add_particle(const particle_layer, const general_particle&);
	void add_particle(const particle_layer, const animated_particle&);
	void add_particle(const particle_layer, const entity_id, const homing_animated_particle&);

	std::size_t count_all_particles() const;

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
		const auto chosen_speed = rng.randval(speed);
		const auto new_velocity = vec2::from_degrees(velocity_degrees) * chosen_speed;
		
		new_particle.set_velocity(new_velocity);
		new_particle.set_position(position);
		new_particle.multiply_size(rng.randval(emission.size_multiplier));
		
		if (emission.should_particles_look_towards_velocity) {
			new_particle.set_rotation(rng.randval(emission.initial_rotation_variation) + velocity_degrees);
		}
		else {
			new_particle.set_rotation(rng.randval(emission.initial_rotation_variation));
		}

		const auto chosen_rotation_speed = rng.randval(emission.rotation_speed);

		new_particle.set_rotation_speed(chosen_rotation_speed);
		new_particle.set_max_lifetime_ms(rng.randval(emission.particle_lifetime_ms));

		if (emission.randomize_acceleration) {
			new_particle.set_acceleration(
				vec2::from_degrees(
					rng.randval(spread) + basic_velocity_degrees
				) * rng.randval(emission.acceleration)
			);
		}

		if (emission.scale_damping_to_velocity) {
			{
				const auto mult = chosen_speed / speed.second;
				new_particle.linear_damping *= mult;
			}

			/* { */
			/* 	const auto mult = chosen_rotation_speed / emission.rotation_speed.second; */
			/* 	new_particle.angular_damping *= mult; */
			/* } */
		}

		return new_particle;
	}

	void integrate_all_particles(
		const cosmos&,
		augs::delta dt,
		const plain_animations_pool& anims,
		const interpolation_system&
	);

	void advance_visible_streams(
		randomization& rng,
		camera_cone,
		const cosmos&,
		const particle_effects_map&,
		const plain_animations_pool&,
		augs::delta dt,
		const interpolation_system&
	);

	void update_effects_from_messages(
		randomization& rng,
		const_logic_step step,
		const particle_effects_map& manager,
		const interpolation_system& interp
	);

	template <class M>
	void draw_particles(
		const M& manager,
		const plain_animations_pool& anims,
		const draw_particles_input input,
		const particle_layer layer
	) const {
		for (const auto& it : general_particles[layer]) {
			it.draw_as_sprite(manager, input);
		}

		for (const auto& it : animated_particles[layer]) {
			it.draw_as_sprite(manager, anims, input);
		}

		for (const auto& cluster : homing_animated_particles[layer]) {
			for (const auto& it : cluster.second) {
				it.draw_as_sprite(manager, anims, input);
			}
		}
	}
};