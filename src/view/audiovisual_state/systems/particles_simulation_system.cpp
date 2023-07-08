#include "augs/templates/container_templates.h"
#include "game/detail/physics/physics_queries.h"

#include "augs/misc/randomization.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "game/messages/start_particle_effect.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/systems/particles_simulation_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "augs/templates/traits/is_nullopt.h"
#include "game/detail/gun/firearm_engine.h"
#include "augs/templates/enum_introspect.h"
#include "view/viewables/particle_types.hpp"
#include "view/viewables/images_in_atlas_map.h"
#include "augs/templates/thread_pool.h"
#include "game/detail/find_absolute_or_local_transform.h"

using emi_inst = particles_simulation_system::emission_instance;

bool emi_inst::is_over() const {
	return stream_lifetime_ms >= stream_max_lifetime_ms;
}

float emi_inst::calc_alivity_mult() const {
	if (stream_max_lifetime_ms == 0.f) {
		return 1.f;
	}

	return stream_lifetime_ms / stream_max_lifetime_ms;
}

float emi_inst::advance_lifetime_get_dt(const augs::delta& delta, const bool stream_infinitely) {
	const auto dt = delta.in_milliseconds();

	if (stream_infinitely) {
		stream_lifetime_ms += dt;
		return dt;
	}

	if (const auto remaining = stream_max_lifetime_ms - stream_lifetime_ms;
		dt >= remaining
	) {
		stream_lifetime_ms = stream_max_lifetime_ms;
		return remaining;
	}

	stream_lifetime_ms += dt;
	return dt;
}

void particles_simulation_system::emission_instance::init_bounds(
	const particles_emission& emission,
	randomization& rng
) {
	{
		const auto var_v = rng.randval(emission.base_speed_variation);
		particle_speed.set(std::max(0.f, emission.base_speed.first - var_v / 2), emission.base_speed.second + var_v / 2);
	}

	source_emission = emission;
	angular_offset = rng.randval(emission.angular_offset);
	spread = rng.randval(emission.spread_degrees);
	particles_per_sec = rng.randval(emission.particles_per_sec);
	swing_spread = rng.randval(emission.swing_spread);
	swings_per_sec = rng.randval(emission.swings_per_sec);

	swing_spread_bound = rng.randval(emission.swing_spread_bound);
	swings_per_sec_bound = rng.randval(emission.swings_per_sec_bound);

	stream_max_lifetime_ms = rng.randval(emission.stream_lifetime_ms);
	stream_particles_to_spawn = rng.randval(emission.num_of_particles_to_spawn_initially);
	swing_speed_change = rng.randval(emission.swing_speed_change_rate);
	swing_spread_change = rng.randval(emission.swing_spread_change_rate);

	fade_when_ms_remaining = rng.randval(emission.fade_when_ms_remaining);

	randomize_spawn_point_within_circle_of_inner_radius = rng.randval(emission.randomize_spawn_point_within_circle_of_inner_radius);
	randomize_spawn_point_within_circle_of_outer_radius = rng.randval(emission.randomize_spawn_point_within_circle_of_outer_radius);

	starting_homing_force = rng.randval(emission.starting_homing_force);
	ending_homing_force = rng.randval(emission.ending_homing_force);

	starting_spawn_circle_size_multiplier = rng.randval(emission.starting_spawn_circle_size_multiplier);
	ending_spawn_circle_size_multiplier = rng.randval(emission.ending_spawn_circle_size_multiplier);
}

particles_simulation_system::emission_instance::emission_instance(
	const particles_emission& emission,
	randomization& rng
) {
	init_bounds(emission, rng);
}

particles_simulation_system::basic_cache::basic_cache(
	const packaged_particle_effect& original,
	const particle_effects_map& manager,
	randomization& rng,
	const special_effects_settings& settings
) : original(original) {
	if (const auto* const source_effect = mapped_or_nullptr(manager, original.input.id)) {
		emission_instances.reserve(source_effect->emissions.size());

		for (auto emission : source_effect->emissions) {
			if (container_full(emission_instances)) {
				break;
			}

			auto& num = emission.num_of_particles_to_spawn_initially;
			num.first = num.first * settings.particle_burst_amount;
			num.second = num.second * settings.particle_burst_amount;

			emission_instances.emplace_back(emission, rng);
			auto& e = emission_instances.back();
			e.stream_particles_to_spawn *= original.input.modifier.scale_amounts;
		}
	}
	else {
		throw effect_not_found {};
	}
}

std::size_t particles_simulation_system::count_particles_on_layer(const particle_layer p) const {
	std::size_t total = 0;

	total += general_particles[p].size();
	total += animated_particles[p].size();

	for (const auto& v : homing_animated_particles[p]) {
		total += v.second.size();
	}

	return total;
}

std::size_t particles_simulation_system::count_all_particles() const {
	std::size_t total = 0;

	total += ::accumulate_sizes(general_particles);
	total += ::accumulate_sizes(animated_particles);

	for (const auto& m : homing_animated_particles) {
		for (const auto& v : m) {
			total += v.second.size();
		}
	}

	return total;
}

void particles_simulation_system::clear() {
	orbital_emissions.clear();
	fire_and_forget_emissions.clear();

	auto clearer = [](auto& arr) { for(auto& v : arr) { v.clear(); } };

	clearer(general_particles);
	clearer(animated_particles);
	clearer(homing_animated_particles);

	firearm_engine_caches.clear();
	continuous_particles_caches.clear();
}

void particles_simulation_system::add_particle(const particle_layer l, const general_particle& p) {
	auto& v = general_particles[l];

	if (container_full(v)) {
		return;
	}

	v.push_back(p);
}

void particles_simulation_system::add_particle(const particle_layer l, const animated_particle& p) {
	auto& v = animated_particles[l];

	if (container_full(v)) {
		return;
	}

	v.push_back(p);
}

void particles_simulation_system::add_particle(const particle_layer l, const entity_id id, const homing_animated_particle& p) {
	homing_animated_particles[l][id].push_back(p);
}

void particles_simulation_system::update_effects_from_messages(
	randomization& rng,
	const const_logic_step step,
	const particle_effects_map& manager,
	const interpolation_system&,
	const special_effects_settings& settings
) {
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
					if (*m != c.original.input.id) {
						return false;
					}	
				}

				return true;
			});
		}
	}

	const auto& events = step.get_queue<messages::start_particle_effect>();

	for (auto& e : events) {
		const auto& start = e.payload.start;

		try {
			if (!start.positioning.target.is_set()) {
				if (!container_full(fire_and_forget_emissions)) {
					fire_and_forget_emissions.emplace_back(
						start.positioning.offset,

						e.payload,
						manager,
						rng,
						settings
					);
				}
			}
			else {
				if (!container_full(orbital_emissions)) {
					orbital_emissions.emplace_back(
						start.positioning,

						e.payload,
						manager,
						rng,
						settings
					);
				}
			}
		}
		catch (const effect_not_found&) {

		}
	}
}

template <class F>
void particles_simulation_system::for_each_particle_in_range(
	const cosmos& cosm, 
	const interpolation_system& interp,
	const int from, 
	const int to, 
	F callback
) {
	int current_i = 0;

	per_particle_layer_t<int> layer_indices;
	fill_range(layer_indices, 0);

	auto maybe_process = [&](const auto layer, auto& range, auto&&... args) {
		auto& li = layer_indices[layer];
		const auto range_n = static_cast<int>(range.size());

		const auto range_left = current_i;
		const auto range_right = current_i + range_n;

		if (range_left == to) {
			/* Already past */
			return;
		}

		if (range_right < from) {
			/* Not yet */
			current_i = range_right;
			li += range_n;
			return;
		}

		const auto trimmed_range_left = std::max(range_left, from);
		const auto trimmed_range_right = std::min(range_right, to);

		{
			const auto real_range_left = trimmed_range_left - range_left;
			const auto real_range_right = trimmed_range_right - range_left;

			li += real_range_left;
			callback(layer, range, li, real_range_left, real_range_right, std::forward<decltype(args)>(args)...);
			li += real_range_right - real_range_left;
		}

		current_i = trimmed_range_right;
	};

	augs::for_each_enum_except_bounds([&](const particle_layer p) {
		maybe_process(p, general_particles[p]);
	});

	augs::for_each_enum_except_bounds([&](const particle_layer p) {
		maybe_process(p, animated_particles[p]);
	});

	augs::for_each_enum_except_bounds([&](const particle_layer p) {
		for (auto& cluster : homing_animated_particles[p]) {
			const auto homing_target = cosm[cluster.first];

			if (homing_target.alive()) {
				const auto homing_transform = cosm[cluster.first].get_viewing_transform(interp);

				maybe_process(p, cluster.second, homing_transform.pos);
			}
		}
	});
}

void particles_simulation_system::remove_dead_particles(const cosmos& cosm) {
	const auto dead_particles_remover = [](auto& container) {
		erase_if(container, [](const auto& a) { return a.is_dead(); });
	};

	for (auto& particle_layer : general_particles) {
		dead_particles_remover(particle_layer);
	}

	for (auto& particle_layer : animated_particles) {
		dead_particles_remover(particle_layer);
	}

	for (auto& particle_layer : homing_animated_particles) {
		erase_if(particle_layer, [&](auto& cluster) {
			const auto homing_target = cosm[cluster.first];

			if (homing_target.dead()) {
				return true;
			}

			dead_particles_remover(cluster.second);

			return cluster.second.empty();
		});
	}
}

void particles_simulation_system::preallocate_particle_buffers(particle_triangle_buffers& buffers) const {
	augs::for_each_enum_except_bounds([&](const particle_layer p) {
		const auto total_on_layer = count_particles_on_layer(p);
		const auto total_triangles_on_layer = total_on_layer * 2;

		{
			auto& target_buffer = buffers.diffuse[p];
			target_buffer.resize(total_triangles_on_layer);
		}

		if (p == particle_layer::NEONING_PARTICLES) {
			auto& target_buffer = buffers.neons;
			target_buffer.resize(total_triangles_on_layer);
		}
	});
}

void particles_simulation_system::integrate_and_draw_all_particles(const integrate_and_draw_all_particles_input in) {
	auto& cosm = in.cosm;
	auto& interp = in.interp;
	auto& output_buffers = in.output;
	auto& anims = in.anims;
	auto& game_images = in.game_images;

	/* 
		This can run in a separate job. 
		Remember that it depends on the cosmos, so it can't advance while the job for this is running. 
	*/

	const auto delta = in.dt.in_seconds();

	auto generic_integrate = [&anims, delta](const particle_layer, auto& range, int, int from_i, const int till_i, auto&&... args) {
		using P = typename remove_cref<decltype(range)>::value_type;

		for (; from_i < till_i; ++from_i) {
			auto& particle = range[from_i];

			if constexpr(std::is_same_v<P, general_particle>) {
				particle.integrate(delta);
			}
			else if constexpr(std::is_same_v<P, animated_particle>) {
				particle.integrate(delta, anims);
			}
			else if constexpr(std::is_same_v<P, homing_animated_particle>) {
				particle.integrate(delta, anims, std::forward<decltype(args)>(args)...);
			}
			else {
				static_assert(always_false_v<P>, "Unimplemented!");
			}
		}
	};

	auto generic_draw = [&output_buffers, &game_images, &anims](const particle_layer p, auto& range, const int layer_index, const int from_i, const int till_i, auto&&...) {
		{
			auto& target_buffer = output_buffers.diffuse[p];

			auto li = layer_index;

			for (int i = from_i; i < till_i; ++i) {
				auto& particle = range[i];

				auto& t1 = target_buffer[2 * li];
				auto& t2 = target_buffer[2 * li + 1];

				particle.template draw_as_sprite<false>(t1, t2, game_images, anims);

				++li;
			}
		}

		if (p == particle_layer::NEONING_PARTICLES) {
			auto& target_buffer = output_buffers.neons;

			auto li = layer_index;

			for (int i = from_i; i < till_i; ++i) {
				auto& particle = range[i];

				auto& t1 = target_buffer[2 * li];
				auto& t2 = target_buffer[2 * li + 1];

				particle.template draw_as_sprite<true>(t1, t2, game_images, anims);

				++li;
			}
		}
	};

	auto integrate_worker = [this, &cosm, &interp, generic_integrate](const int from, const int to) {
		for_each_particle_in_range(
			cosm,
			interp,
			from,
			to, 
			generic_integrate
		);
	};

	auto draw_worker = [this, &cosm, &interp, generic_draw](const int from, const int to) {
		for_each_particle_in_range(
			cosm,
			interp,
			from, 
			to, 
			generic_draw
		);
	};

	const auto total_n = static_cast<int>(count_all_particles());

	if (total_n == 0) {
		return;
	}

	const auto max_per_job_n = std::max(1, in.max_particles_in_single_job);
	const auto jobs_n = 1 + total_n / max_per_job_n;
	const auto per_job_n = total_n / jobs_n;

	for (int i = 0; i < jobs_n; ++i) {
		const bool is_last = i == jobs_n - 1;
		const auto from = i * per_job_n;
		const auto to = is_last ? total_n : from + per_job_n;

		in.pool.enqueue([from, to, integrate_worker, draw_worker]() {
			integrate_worker(from, to);
			draw_worker(from, to);
		});
	}
}

template <class Component, class Caches, class EffectProvider>
void update_component_related_cache(
	randomization& rng,
	const particle_effects_map& manager,
	const cosmos& cosm,
	Caches& caches,
	EffectProvider effect_provider,
	const special_effects_settings& settings
) {
	cosm.for_each_having<Component>(
		[&](const auto& typed_handle) {
			const auto id = typed_handle.get_id().to_unversioned();

			if (const auto item = typed_handle.template find<components::item>()) {
				if (const auto slot = typed_handle.get_current_slot(); slot.alive()) {
					/* 
						Disable effects for holstered items and items on the body,
						to prevent giving away of the positions.
					*/

					if (!slot.is_hand_slot()) {
						caches.erase(id);
						return;
					}

					if (typed_handle.find_colliders_connection() == nullptr) {
						caches.erase(id);
						return;
					}
				}
			}

			if (const auto particles = effect_provider(typed_handle)) {
				if (auto* const existing = mapped_or_nullptr(caches, id)) {
					existing->cache.original = *particles;
				}
				else {
					try {
						caches.try_emplace(id, particles_simulation_system::continuous_particles_cache { { particles->start.positioning, *particles, manager, rng, settings }, { typed_handle.get_name() } });
					}
					catch (const particles_simulation_system::effect_not_found&) {

					}
				}
			}
			else {
				caches.erase(id);
			}
		}
	);

	erase_if(caches, [&](auto& it) {
		return cosm[it.first].dead();
	});
}

void particles_simulation_system::advance_visible_streams(
	randomization& rng,
	const camera_cone queried_cone, 
	const special_effects_settings& settings,
	const cosmos& cosm,
	const particle_effects_map& manager,
	const plain_animations_pool& anims,
	const augs::delta& delta,
	const interpolation_system& interp
) {
	const auto dt_secs = delta.in_seconds();

	auto advance_emissions = [&](
		emission_instances_type& instances, 
		const transformr current_transform,
		const bool visible_in_camera,
		const packaged_particle_effect& effect,
		const vec2 chased_velocity = vec2::zero
	) {
		auto particles_checked_cone = queried_cone;
		particles_checked_cone.eye.zoom /= 2.f;

		const auto particles_cam_aabb = particles_checked_cone.get_visible_world_rect_aabb();

		const auto& modifier = effect.input.modifier;
		const auto homing_target = effect.start.homing_target;
		const auto infinitely = effect.start.stream_infinitely;

		for (auto& instance : instances) {
			const auto stream_alivity_mult = std::fmod(instance.calc_alivity_mult(), 1.0f);
			const auto stream_delta = instance.advance_lifetime_get_dt(delta, infinitely);

			if (!visible_in_camera) {
				continue;
			}

			const auto total_amount_mult = modifier.scale_amounts * settings.particle_stream_amount;

			auto new_particles_to_spawn_by_time = 
				(instance.particles_per_sec * total_amount_mult) * 
				(stream_delta / 1000.f)
			;

			instance.stream_particles_to_spawn += new_particles_to_spawn_by_time;

			instance.swings_per_sec += rng.randval_h(instance.swing_speed_change);
			instance.swing_spread += rng.randval_h(instance.swing_spread_change);

			auto clamp = [](auto& val, const auto bound) {
				val = std::clamp(val, bound.first, bound.second);
			};

			if (instance.swing_spread_bound.second > 0) {
				clamp(instance.swing_spread, instance.swing_spread_bound);
			}
			if (instance.swings_per_sec_bound.second > 0) {
				clamp(instance.swings_per_sec, instance.swings_per_sec_bound);
			}

#if TODO
			const auto segment_length = existence.distribute_within_segment_of_length;
#endif
			const float segment_length = 0.f;

			const vec2 segment_A = current_transform.pos + vec2::from_degrees(current_transform.rotation + 90).set_length(segment_length / 2);
			const vec2 segment_B = current_transform.pos - vec2::from_degrees(current_transform.rotation + 90).set_length(segment_length / 2);
			
			const auto homing_target_pos = cosm[homing_target].alive() ? cosm[homing_target].get_viewing_transform(interp).pos : vec2();

			const auto to_spawn = static_cast<int>(instance.stream_particles_to_spawn);

			for (int i = 0; i < to_spawn; ++i) {
				const float t = (static_cast<float>(i) / to_spawn);
				const float time_elapsed = (1.f - t) * dt_secs;

				vec2 final_particle_position = augs::interp(segment_A, segment_B, rng.randval(0.f, 1.f));
				
				const auto& emission = instance.source_emission;

				if (modifier.box != vec2::zero) {
					const auto rx = rng.randval(0.f, 1.f) - 0.5f;
					const auto ry = rng.randval(0.f, 1.f) - 0.5f;

					final_particle_position += modifier.box * vec2(rx, ry).rotate(current_transform.rotation);
				}
				else {
					auto considered_inner = instance.randomize_spawn_point_within_circle_of_inner_radius;
					auto considered_outer = instance.randomize_spawn_point_within_circle_of_outer_radius;

					if (modifier.radius != 0.0f) {
						considered_inner = 0.0f;
						considered_outer = modifier.radius;
					}

					if (considered_inner > 0.f || considered_outer > 0.f) {
						const auto size_mult = augs::interp(
							instance.starting_spawn_circle_size_multiplier,
							instance.ending_spawn_circle_size_multiplier,
							emission.use_sqrt_to_ease_spawn_circle ? std::sqrt(stream_alivity_mult) : stream_alivity_mult
						);
						
						final_particle_position += rng.random_point_in_ring(
							size_mult * considered_inner,
							size_mult * considered_outer
						);
					}
				}

				const bool visible_in_camera = particles_cam_aabb.hover(final_particle_position);

				if (!visible_in_camera) {
					continue;
				}

				/* MSVC ICE workaround */
				auto& _rng = rng;

				const auto spawner = [&](auto dummy) {
					using spawned_particle_type = decltype(dummy);

					auto particle = this->spawn_particle<spawned_particle_type>(
						_rng,
						instance.angular_offset,
						instance.particle_speed,
						final_particle_position,
						current_transform.rotation + instance.swing_spread * static_cast<float>(std::sin((instance.stream_lifetime_ms / 1000.f) * 2 * PI<float> * instance.swings_per_sec)),
						instance.spread,
						emission,
						chased_velocity
					);

					return ::apply_to_particle(modifier, particle);
				};

				if (emission.has<general_particle>()) {
					auto new_general = spawner(general_particle());
					new_general.integrate(time_elapsed);
					add_particle(emission.target_layer, new_general);
				}

				if (emission.has<animated_particle>()) {
					auto new_animated = spawner(animated_particle());
					new_animated.integrate(time_elapsed, anims);
					add_particle(emission.target_layer, new_animated);
				}

				if (emission.has<homing_animated_particle>()) {
					auto new_homing_animated = spawner(homing_animated_particle());

					new_homing_animated.homing_force = augs::interp(
						instance.starting_homing_force,
						instance.ending_homing_force,
						stream_alivity_mult
					);

					new_homing_animated.integrate(time_elapsed, anims, homing_target_pos);
					add_particle(emission.target_layer, homing_target, new_homing_animated);
				}
			}

			/* Leave only the fractional part */
			instance.stream_particles_to_spawn -= static_cast<int>(instance.stream_particles_to_spawn);
		}
	};

	{
		auto checked_cone = queried_cone;
		checked_cone.eye.zoom /= 2.f;

		const auto cam_aabb = checked_cone.get_visible_world_rect_aabb();

		erase_if(fire_and_forget_emissions, [&](auto& c) {
			const auto where = c.transform;
			const bool visible_in_camera = cam_aabb.hover(where.pos);

			advance_emissions(c.emission_instances, where, visible_in_camera, c.original);
			return c.is_over();
		});

		erase_if(orbital_emissions, [&](auto& c) {
			const auto chase = c.chasing;
			const auto where = find_transform(chase, cosm, interp);
			
			if (where == std::nullopt) {
				return true;
			}

			const bool visible_in_camera = cam_aabb.hover(where->pos);

			auto chased_velocity = vec2::zero;

			if (chase.chase_velocity) {
				if (chase.target.is_set()) {
					if (const auto target_handle = cosm[chase.target]) {
						chased_velocity = target_handle.get_effective_velocity();
					}
				}
			}

			advance_emissions(c.emission_instances, *where, visible_in_camera, c.original, chased_velocity);
			return c.is_over();
		});

		erase_if(firearm_engine_caches, [&](auto& it) {
			auto& c = it.second.cache;

			const auto chase = c.chasing;
			const auto where = find_transform(chase, cosm, interp);

			if (where == std::nullopt) {
				return true;
			}

			if (const auto subject = cosm[it.first]) {
				if (subject.get_name() != it.second.recorded.name) {
					return true;
				}
			}

			const bool visible_in_camera = cam_aabb.hover(where->pos);

			advance_emissions(c.emission_instances, *where, visible_in_camera, c.original);
			return false;
		});
		
		erase_if(continuous_particles_caches, [&](auto& it) { 
			auto& c = it.second.cache;
			const auto chase = c.chasing;

			auto where = find_transform(chase, cosm, interp);

			if (where == std::nullopt) {
				return true;
			}

			if (const auto subject = cosm[it.first]) {
				if (subject.get_name() != it.second.recorded.name) {
					return true;
				}

				if (auto particles = subject.template find<invariants::continuous_particles>()) {
					if (particles->effect_id != it.second.cache.original.input.id) {
						return true;
					}
				}
				else {
					return true;
				}
			}

			const auto displacement = [&]() {
				if (const auto subject = cosm[it.first]) {
					return subject.template dispatch_on_having_all_ret<components::continuous_particles>([](const auto& typed_subject) {
						if constexpr(is_nullopt_v<decltype(typed_subject)>) {
							return transformr();
						}
						else {
							return transformr(typed_subject.template get<components::continuous_particles>().wandering_state.current, 0);
						}
					});
				}

				return transformr();
			}();

			*where += displacement;
			const bool visible_in_camera = cam_aabb.hover(where->pos);

			auto chased_velocity = vec2::zero;

			if (chase.chase_velocity) {
				if (chase.target.is_set()) {
					if (const auto target_handle = cosm[chase.target]) {
						chased_velocity = target_handle.get_effective_velocity();
					}
				}
			}

			advance_emissions(c.emission_instances, *where, visible_in_camera, c.original, chased_velocity);
			return false;
		});
	}

	update_component_related_cache<components::gun>(
		rng,
		manager,
		cosm,
		firearm_engine_caches,
		[](const auto h) {
			return ::calc_firearm_engine_particles(h);
		},
		settings
	);

	update_component_related_cache<components::continuous_particles>(
		rng,
		manager,
		cosm,
		continuous_particles_caches,
		[](const auto h) -> std::optional<packaged_particle_effect> {
			const auto& continuous_particles = h.template get<invariants::continuous_particles>();
			const auto& cp = h.template get<components::continuous_particles>();

			packaged_particle_effect particles;

			particles.start = particle_effect_start_input::at_entity(h);
			particles.start.stream_infinitely = true;

			particles.input.id = continuous_particles.effect_id;
			particles.input.modifier = cp.modifier;

			if (!particles.input.id.is_set()) {
				return std::nullopt;
			}

			return std::make_optional(particles);
		},
		settings
	);
}
