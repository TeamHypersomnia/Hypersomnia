#include "augs/templates/container_templates.h"
#include "game/detail/physics/physics_queries.h"

#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "game/messages/start_particle_effect.h"

#include "view/viewables/all_viewables_declarations.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/systems/particles_simulation_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void particles_simulation_system::clear() {
	orbital_emissions.clear();
	fire_and_forget_emissions.clear();
}

void particles_simulation_system::clear_dead_entities(const cosmos& new_cosmos) {
	erase_if(orbital_emissions, [&](const auto& it) {
		const auto target = new_cosmos[it.chasing.target];
		return target.dead() || !target.find_logic_transform().has_value();
	});
}

void particles_simulation_system::add_particle(const render_layer l, const general_particle& p) {
	general_particles[l].push_back(p);
}

void particles_simulation_system::add_particle(const render_layer l, const animated_particle& p) {
	animated_particles[l].push_back(p);
}

void particles_simulation_system::add_particle(const render_layer l, const entity_id id, const homing_animated_particle& p) {
	homing_animated_particles[l][id].push_back(p);
}

void particles_simulation_system::update_effects_from_messages(
	const const_logic_step step,
	const particle_effects_map& manager,
	const interpolation_system& interp
) {
	thread_local randomization rng;

	{
		const auto& events = step.get_queue<messages::stop_particle_effect>();

		for (auto& e : events) {
			erase_if(orbital_emissions, [&e](const orbital_cache& c){	
				if (const auto& m = e.match_chased_subject) {
					if (*m != c.chasing.target) {
						return false;
					}	
				}

				if (const auto m = e.match_orbit_offset) {
					if (*m != c.chasing.offset.pos) {
						return false;
					}	
				}

				if (const auto m = e.match_effect_id) {
					if (*m != c.original_effect.id) {
						return false;
					}	
				}

#if MORE_LOGS
				if (c.original_effect.id == assets::particle_effect_id::HEALTH_DAMAGE_SPARKLES) {
					LOG("Removing health sparkles");
				}
#endif

				return true;
			});
		}
	}

	const auto& events = step.get_queue<messages::start_particle_effect>();

	auto instantiate_emission = [&](const particles_emission& emission) {
		emission_instance instance;

		const auto var_v = rng.randval(emission.base_speed_variation);
		//LOG("V: %x", var_v);
		instance.particle_speed.set(std::max(0.f, emission.base_speed.first - var_v / 2), emission.base_speed.second + var_v / 2);
		//LOG("Vl: %x Vu: %x", instance.velocity.first, instance.velocity.second);

		instance.source_emission = emission;
		instance.enable_streaming = true;
		instance.stream_lifetime_ms = 0.f;
		instance.angular_offset = rng.randval(emission.angular_offset);
		instance.spread = rng.randval(emission.spread_degrees);
		instance.particles_per_sec = rng.randval(emission.particles_per_sec);
		instance.swing_spread = rng.randval(emission.swing_spread);
		instance.swings_per_sec = rng.randval(emission.swings_per_sec);

		instance.swing_spread_bound = rng.randval(emission.swing_spread_bound);
		instance.swings_per_sec_bound = rng.randval(emission.swings_per_sec_bound);

		instance.stream_max_lifetime_ms = rng.randval(emission.stream_lifetime_ms);
		instance.stream_particles_to_spawn = rng.randval(emission.num_of_particles_to_spawn_initially);
		instance.swing_speed_change = rng.randval(emission.swing_speed_change_rate);
		instance.swing_spread_change = rng.randval(emission.swing_spread_change_rate);

		instance.fade_when_ms_remaining = rng.randval(emission.fade_when_ms_remaining);

		instance.randomize_spawn_point_within_circle_of_inner_radius = rng.randval(emission.randomize_spawn_point_within_circle_of_inner_radius);
		instance.randomize_spawn_point_within_circle_of_outer_radius = rng.randval(emission.randomize_spawn_point_within_circle_of_outer_radius);

		instance.starting_homing_force = rng.randval(emission.starting_homing_force);
		instance.ending_homing_force = rng.randval(emission.ending_homing_force);

		instance.starting_spawn_circle_size_multiplier = rng.randval(emission.starting_spawn_circle_size_multiplier);
		instance.ending_spawn_circle_size_multiplier = rng.randval(emission.ending_spawn_circle_size_multiplier);

		return instance;
	};

	for (auto& e : events) {
		const auto& effect = e.effect;
		const auto& start = e.start;

		if (const auto* const source_effect = mapped_or_nullptr(manager, effect.id)) {
			auto& instances = [&]() -> emission_instances& {
				if (!start.positioning.target.is_set()) {
					faf_cache c;
					c.transform = start.positioning.offset;
					fire_and_forget_emissions.push_back(c);

					return fire_and_forget_emissions.back().emission_instances;
				}

				orbital_cache c;
				c.chasing = start.positioning;
				c.original_effect = effect;
				orbital_emissions.push_back(c);

#if MORE_LOGS
				if (effect.id == assets::particle_effect_id::HEALTH_DAMAGE_SPARKLES) {
					LOG("Adding health sparkles");
				}
#endif

				return orbital_emissions.back().emission_instances;
			}();

			for (auto emission : source_effect->emissions) {
				emission.apply_modifier(effect.modifier);

				auto new_instance = instantiate_emission(emission);
				new_instance.homing_target = start.homing_target;

				instances.push_back(new_instance);
			}
		}
	}
}

void particles_simulation_system::integrate_all_particles(
	const cosmos& cosmos,
	const augs::delta delta,
	const animations_pool& anims,
	const interpolation_system& interp
) {
	const auto dead_particles_remover = [](auto& container) {
		erase_if(container, [](const auto& a) { return a.is_dead(); });
	};

	for (auto& particle_layer : general_particles) {
		dead_particles_remover(particle_layer);

		for (auto& p : particle_layer) {
			p.integrate(delta.in_seconds(), anims);
		}
	}

	for (auto& particle_layer : animated_particles) {
		dead_particles_remover(particle_layer);

		for (auto& p : particle_layer) {
			p.integrate(delta.in_seconds(), anims);
		}
	}

	for (auto& particle_layer : homing_animated_particles) {
		erase_if(particle_layer, [&](auto& cluster) {
			const auto homing_target = cosmos[cluster.first];

			if (homing_target.alive()) {
				dead_particles_remover(cluster.second);

				const auto homing_transform = cosmos[cluster.first].get_viewing_transform(interp);

				for (auto& p : cluster.second) {
					p.integrate(delta.in_seconds(), homing_transform.pos);
				}

				return cluster.second.empty();
			}

			return true;
		});
	}
}

void particles_simulation_system::advance_visible_streams(
	const camera_cone current_cone, 
	const vec2 screen_size,
	const cosmos& cosmos,
	const particle_effects_map& manager,
	const augs::delta delta,
	const interpolation_system& interp
) {
	thread_local randomization rng;

	auto advance_emissions = [&](
		emission_instances& instances, 
		const components::transform current_transform
	) {
		for (auto& instance : instances) {
			const auto stream_alivity_mult = 
				instance.stream_max_lifetime_ms == 0.f ? 1.f : instance.stream_lifetime_ms / instance.stream_max_lifetime_ms
			;

			const float stream_delta = std::min(delta.in_milliseconds(), instance.stream_max_lifetime_ms - instance.stream_lifetime_ms);
			const auto& emission = instance.source_emission;

			instance.stream_lifetime_ms += stream_delta;

			auto new_particles_to_spawn_by_time = instance.particles_per_sec * (stream_delta / 1000.f);

			instance.stream_particles_to_spawn += new_particles_to_spawn_by_time;

			instance.swings_per_sec += rng.randval(-instance.swing_speed_change, instance.swing_speed_change);
			instance.swing_spread += rng.randval(-instance.swing_spread_change, instance.swing_spread_change);

			auto clamp = [](auto& val, const auto bound) {
				val = std::clamp(val, bound.first, bound.second);
			};

			if (instance.swing_spread_bound.second > 0) {
				clamp(instance.swing_spread, instance.swing_spread_bound);
			}
			if (instance.swings_per_sec_bound.second > 0) {
				clamp(instance.swings_per_sec, instance.swings_per_sec_bound);
			}

			const int to_spawn = static_cast<int>(std::floor(instance.stream_particles_to_spawn));

#if TODO
			const auto segment_length = existence.distribute_within_segment_of_length;
#endif
			const float segment_length = 0.f;

			const vec2 segment_A = current_transform.pos + vec2::from_degrees(current_transform.rotation + 90).set_length(segment_length / 2);
			const vec2 segment_B = current_transform.pos - vec2::from_degrees(current_transform.rotation + 90).set_length(segment_length / 2);
			
			const auto homing_target_pos = cosmos[instance.homing_target].alive() ? cosmos[instance.homing_target].get_viewing_transform(interp).pos : vec2();

			for (int i = 0; i < to_spawn; ++i) {
				const float t = (static_cast<float>(i) / to_spawn);
				const float time_elapsed = (1.f - t) * delta.in_seconds();

				vec2 final_particle_position = augs::interp(segment_A, segment_B, rng.randval(0.f, 1.f));
				
				if (
					instance.randomize_spawn_point_within_circle_of_inner_radius > 0.f
					|| instance.randomize_spawn_point_within_circle_of_outer_radius > 0.f
				) {
					const auto size_mult = augs::interp(
						instance.starting_spawn_circle_size_multiplier,
						instance.ending_spawn_circle_size_multiplier,
						stream_alivity_mult
					);
					
					final_particle_position += rng.random_point_in_ring(
						size_mult * instance.randomize_spawn_point_within_circle_of_inner_radius,
						size_mult * instance.randomize_spawn_point_within_circle_of_outer_radius
					);
				}

				/* MSVC ICE workaround */
				auto& _rng = rng;

				const auto spawner = [&](auto dummy) {
					using spawned_particle_type = decltype(dummy);

					return this->spawn_particle<spawned_particle_type>(
						_rng,
						instance.angular_offset,
						instance.particle_speed,
						final_particle_position,
						current_transform.rotation + instance.swing_spread * static_cast<float>(sin((instance.stream_lifetime_ms / 1000.f) * 2 * PI<float> * instance.swings_per_sec)),
						instance.spread,
						emission
					);
				};

				if (emission.get_definitions<general_particle>().size() > 0) {
					auto new_general = spawner(general_particle());
					new_general.integrate(time_elapsed);
					add_particle(emission.target_render_layer, new_general);
				}

				if (emission.get_definitions<animated_particle>().size() > 0)
				{
					auto new_animated = spawner(animated_particle());
					new_animated.integrate(time_elapsed);
					add_particle(emission.target_render_layer, new_animated);
				}

				if (emission.get_definitions<homing_animated_particle>().size() > 0)
				{
					auto new_homing_animated = spawner(homing_animated_particle());

					new_homing_animated.homing_force = augs::interp(
						instance.starting_homing_force,
						instance.ending_homing_force,
						stream_alivity_mult
					);

					new_homing_animated.integrate(time_elapsed, homing_target_pos);
					add_particle(emission.target_render_layer, instance.homing_target, new_homing_animated);
				}

				instance.stream_particles_to_spawn -= 1.f;
			}
		}
	};

	{
		auto checked_cone = current_cone;
		checked_cone.zoom /= 2.5f;

		for (auto& c : fire_and_forget_emissions) { 
			const auto where = c.transform;

			if (!checked_cone.get_visible_world_rect_aabb(screen_size).hover(where.pos)) {
				continue;
			}

			advance_emissions(c.emission_instances, where);
		}

		for (auto& c : orbital_emissions) { 
			const auto chase = c.chasing;
			const auto where = find_transform(chase, cosmos, interp);

			if (!where) {
				continue;
			}

			if (!checked_cone.get_visible_world_rect_aabb(screen_size).hover(where->pos)) {
				continue;
			}

			advance_emissions(c.emission_instances, *where);
		}
	}

	erase_if(fire_and_forget_emissions, [](const faf_cache& c){ return c.is_over(); });
	erase_if(orbital_emissions, [](const orbital_cache& c){ return c.is_over(); });
}
