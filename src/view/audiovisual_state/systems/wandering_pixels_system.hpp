#pragma once
#include "view/audiovisual_state/systems/wandering_pixels_system.h"
#include "game/cosmos/typed_entity_handle_declaration.h"
#include "augs/math/steering.h"

template <class E>
void wandering_pixels_system::allocate_cache_for(E id) {
	const auto unversioned = id.to_unversioned();
	
	if (!found_in(per_entity_cache, unversioned)) {
		per_entity_cache[unversioned] = {};
	}
}
	
template <class E>
wandering_pixels_system::cache* wandering_pixels_system::find_cache(const E id) {
	const auto unversioned = id.to_unversioned();
	return mapped_or_nullptr(per_entity_cache, unversioned);
}

template <class E>
const wandering_pixels_system::cache* wandering_pixels_system::find_cache(const E id) const {
	const auto unversioned = id.to_unversioned();
	return mapped_or_nullptr(per_entity_cache, unversioned);
}

template <class E>
void wandering_pixels_system::advance_for(
	const E it, 
	const augs::delta dt
) {
	thread_local randomization rng;

	const auto dt_secs = dt.in_seconds();
	const auto dt_ms = dt.in_milliseconds();

	auto maybe_cache = find_cache(it.get_id());

	if (maybe_cache == nullptr) {
		return;
	}

	auto& cache = *maybe_cache;

	auto& used_rng = rng;

	const auto& wandering = it.template get<components::wandering_pixels>();
	const auto& wandering_def = it.template get<invariants::wandering_pixels>();

	const auto& cosm = it.get_cosmos();

	const auto anim = cosm.get_logical_assets().find(wandering_def.animation_id);

	if (anim == nullptr) {
		return;
	}

	const auto total_animation_duration = ::calc_total_duration(anim->frames);
	auto current_reach = xywh();

	if (const auto aabb = it.find_aabb()) {
		current_reach = *aabb;
	}

	auto reset_positions = [&]() {
		for (auto& p : cache.particles) {
			p.pos.set(
				current_reach.x + used_rng.randval(0u, static_cast<unsigned>(current_reach.w)), 
				current_reach.y + used_rng.randval(0u, static_cast<unsigned>(current_reach.h))
			);

			p.current_lifetime_ms = used_rng.randval(0.f, total_animation_duration);
		}
	};

	if (cache.recorded_particle_count != wandering.num_particles) {
		cache.particles.resize(wandering.num_particles);
		cache.recorded_particle_count = wandering.num_particles;
		reset_positions();
	}

	if (cache.recorded_reach != current_reach) {
		/* refresh_cache */ 
		reset_positions();
		cache.recorded_reach = current_reach;
	}

	const auto max_direction_ms = wandering_def.max_direction_ms;
	const auto direction_interp_ms  = wandering_def.direction_interp_ms;

	for (auto& p : cache.particles) {
		p.current_lifetime_ms += dt_ms + dt_ms * (p.direction_ms_left / max_direction_ms) + dt_ms * p.current_direction.radians();

		if (p.direction_ms_left <= 0.f) {
			p.direction_ms_left = static_cast<float>(used_rng.randval(max_direction_ms, max_direction_ms + 800u));

			p.current_direction = p.current_direction.perpendicular_cw();

			const auto dir = p.current_direction;
			const auto reach = current_reach;

			float chance_to_flip = 0.f;

			p.current_velocity = static_cast<float>(used_rng.randval(wandering_def.base_velocity));

			if (dir.x > 0) {
				chance_to_flip = (p.pos.x - reach.x) / reach.w;
			}
			else if (dir.x < 0) {
				chance_to_flip = 1.f - (p.pos.x - reach.x) / reach.w;
			}
			if (dir.y > 0) {
				chance_to_flip = (p.pos.y - reach.y) / reach.h;
			}
			else if (dir.y < 0) {
				chance_to_flip = 1.f - (p.pos.y - reach.y) / reach.h;
			}

			if (chance_to_flip < 0) {
				chance_to_flip = 0;
			}

			if (chance_to_flip > 1) {
				chance_to_flip = 1;
			}

			if (used_rng.randval(0u, 100u) <= chance_to_flip * 100.f) {
				p.current_direction = -p.current_direction;
			}
		}
		else {
			p.direction_ms_left -= dt_ms;
		}

		vec2 considered_direction;

		if (p.direction_ms_left <= direction_interp_ms) {
			considered_direction = augs::interp(vec2(), p.current_direction, p.direction_ms_left / direction_interp_ms);
		}
		else if (p.direction_ms_left >= max_direction_ms - direction_interp_ms) {
			considered_direction = augs::interp(vec2(), p.current_direction, (max_direction_ms - p.direction_ms_left) / direction_interp_ms);
		}
		else {
			considered_direction = p.current_direction;
		}

		const auto vel = p.current_velocity;

		auto prev_pos = p.pos;
		p.pos += considered_direction * vel * dt_secs;

		const auto sin_secs = static_cast<float>(std::sin(p.current_lifetime_ms / 1000));
		const auto cos_secs = static_cast<float>(std::cos(p.current_lifetime_ms / 1000));

		if (considered_direction.x > 0) {
			p.pos.y += considered_direction.x * sin_secs * vel * dt_secs * 1.2f;
		}
		else if (considered_direction.x < 0) {
			p.pos.y -= -considered_direction.x * sin_secs * vel * dt_secs * 1.2f;
		}
		if (considered_direction.y > 0) {
			p.pos.x += considered_direction.y * cos_secs * vel * dt_secs * 1.2f;
		}
		else if (considered_direction.y < 0) {
			p.pos.x -= -considered_direction.y * cos_secs * vel * dt_secs * 1.2f;
		}

		//p.pos.x += cos(global_time_seconds) * 20 * dt_secs * 1.2;

		if (wandering.force_particles_within_bounds) {
			const auto velocity = p.pos - prev_pos;

			auto steer_result = augs::steer_to_avoid_edges(
				velocity,
				p.pos,
				current_reach.get_vertices(),
				current_reach.get_center(),
				30.f,
				1.f
			);

			p.pos += steer_result.seek_vector;
		}
	}
}
