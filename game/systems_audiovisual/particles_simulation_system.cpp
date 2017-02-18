#include "particles_simulation_system.h"

#include "augs/misc/randomization.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/position_copying_component.h"

#include "augs/templates/container_templates.h"
#include "game/detail/particle_types.h"

void particles_simulation_system::draw(
	augs::vertex_triangle_buffer& target_buffer,
	const render_layer layer,
	const camera_cone camera,
	const renderable_drawing_type drawing_type
) const {
	components::sprite::drawing_input general_input(target_buffer);
	general_input.camera = camera;
	general_input.drawing_type = drawing_type;

	for (const auto& it : get_particles<general_particle>()[layer]) {
		it.draw(general_input);
	}

	for (const auto& it : get_particles<animated_particle>()[layer]) {
		it.draw(general_input);
	}
}

void particles_simulation_system::erase_caches_for_dead_entities(const cosmos& new_cosmos) {
	std::vector<entity_id> to_erase;

	for (const auto it : per_entity_cache) {
		if (new_cosmos[it.first].dead()) {
			to_erase.push_back(it.first);
		}
	}

	for (const auto it : to_erase) {
		per_entity_cache.erase(it);
	}
}

particles_simulation_system::cache& particles_simulation_system::get_cache(const const_entity_handle id) {
	return per_entity_cache[id.get_id()];
}

void particles_simulation_system::advance_visible_streams_and_all_particles(
	camera_cone cone, 
	const cosmos& cosmos, 
	const augs::delta delta, 
	const interpolation_system& interp
) {
	const auto dead_particles_remover = [](auto& container) {
		erase_remove(container, [](const auto& a) { return a.is_dead(); });
	};

	for (auto& particle_layer : get_particles<general_particle>()) {
		for (auto& p : particle_layer) {
			p.integrate(delta.in_seconds());
		}

		dead_particles_remover(particle_layer);
	}

	for (auto& particle_layer : get_particles<animated_particle>()) {
		for (auto& p : particle_layer) {
			p.integrate(delta.in_seconds());
		}

		dead_particles_remover(particle_layer);
	}

	cone.visible_world_area *= 2.5f;

	static thread_local std::vector<unversioned_entity_id> targets;
	targets.clear();

	cosmos.systems_temporary.get<dynamic_tree_system>().determine_visible_entities_from_camera(
		targets,
		cone, 
		components::dynamic_tree_node::tree_type::PARTICLE_EXISTENCES
	);

	for (const auto it_id : targets) {
		const auto it = cosmos[it_id];

		auto& cache = get_cache(it);
		const auto& existence = it.get<components::particles_existence>();

		const bool should_rebuild_cache = cache.recorded_existence != existence;

		if (should_rebuild_cache) {
			randomization rng = existence.rng_seed;
			cache.recorded_existence = existence;
			cache.emission_instances.clear();

			for (auto emission : (*existence.input.effect)) {
				emission.apply_modifier(existence.input.modifier);

				cache.emission_instances.push_back(emission_instance());
				auto& target_stream = *cache.emission_instances.rbegin();

				const auto var_v = rng.randval(emission.base_speed_variation);
				//LOG("V: %x", var_v);
				target_stream.particle_speed.set(std::max(0.f, emission.base_speed.first - var_v / 2), emission.base_speed.second + var_v / 2);
				//LOG("Vl: %x Vu: %x", target_stream.velocity.first, target_stream.velocity.second);

				target_stream.stream_info = emission;
				target_stream.enable_streaming = true;
				target_stream.stream_lifetime_ms = 0.f;
				target_stream.angular_offset = rng.randval(emission.angular_offset);
				target_stream.target_spread = rng.randval(emission.spread_degrees);
				target_stream.target_particles_per_sec = rng.randval(emission.particles_per_sec);
				target_stream.swing_spread = rng.randval(emission.swing_spread);
				target_stream.swings_per_sec = rng.randval(emission.swings_per_sec);

				target_stream.min_swing_spread = rng.randval(emission.min_swing_spread);
				target_stream.min_swings_per_sec = rng.randval(emission.min_swings_per_sec);
				target_stream.max_swing_spread = rng.randval(emission.max_swing_spread);
				target_stream.max_swings_per_sec = rng.randval(emission.max_swings_per_sec);

				target_stream.stream_max_lifetime_ms = rng.randval(emission.stream_duration_ms);
				target_stream.stream_particles_to_spawn = rng.randval(emission.num_of_particles_to_spawn_initially);
				target_stream.swing_speed_change = rng.randval(emission.swing_speed_change_rate);
				target_stream.swing_spread_change = rng.randval(emission.swing_spread_change_rate);

				target_stream.fade_when_ms_remaining = rng.randval(emission.fade_when_ms_remaining);
			}
		}

		const auto transform = it.get_viewing_transform(interp) + existence.current_displacement;
		randomization rng = cosmos.get_rng_seed_for(it) + cosmos.get_total_steps_passed();

		bool should_destroy = true;

		for (auto& instance : cache.emission_instances) {
			const float stream_delta = std::min(delta.in_milliseconds(), instance.stream_max_lifetime_ms - instance.stream_lifetime_ms);

			instance.stream_lifetime_ms += stream_delta;

			if (instance.stream_lifetime_ms > instance.stream_max_lifetime_ms) {
				continue;
			}

			auto new_particles_to_spawn_by_time = instance.target_particles_per_sec * (stream_delta / 1000.f);

			instance.stream_particles_to_spawn += new_particles_to_spawn_by_time;

			instance.swings_per_sec += rng.randval(-instance.swing_speed_change, instance.swing_speed_change);
			instance.swing_spread += rng.randval(-instance.swing_spread_change, instance.swing_spread_change);

			if (instance.max_swing_spread > 0) {
				augs::clamp(instance.swing_spread, instance.min_swing_spread, instance.max_swing_spread);
			}
			if (instance.max_swings_per_sec > 0) {
				augs::clamp(instance.swings_per_sec, instance.min_swings_per_sec, instance.max_swings_per_sec);
			}

			const int to_spawn = static_cast<int>(std::floor(instance.stream_particles_to_spawn));

			const auto segment_length = existence.distribute_within_segment_of_length;
			const vec2 segment_A = transform.pos + vec2().set_from_degrees(transform.rotation + 90).set_length(segment_length / 2);
			const vec2 segment_B = transform.pos - vec2().set_from_degrees(transform.rotation + 90).set_length(segment_length / 2);

			for_each_particle_type([&](auto dummy) {
				typedef decltype(dummy) spawned_particle_type;

				if (instance.stream_info.get_templates<spawned_particle_type>().size() > 0) {
					for (int i = 0; i < to_spawn; ++i) {
						const float t = (static_cast<float>(i) / to_spawn);
						const float time_elapsed = (1.f - t) * delta.in_seconds();

						const vec2 segment_position = augs::interp(segment_A, segment_B, rng.randval(0.f, 1.f));

						spawn_particle<spawned_particle_type>(
							rng,
							instance.angular_offset,
							instance.particle_speed,
							segment_position,
							transform.rotation + instance.swing_spread * static_cast<float>(sin((instance.stream_lifetime_ms / 1000.f) * 2 * PI_f * instance.swings_per_sec)),
							instance.target_spread,
							instance.stream_info
							).integrate(time_elapsed);

						instance.stream_particles_to_spawn -= 1.f;
					}
				}
			});
		}
	}
}
