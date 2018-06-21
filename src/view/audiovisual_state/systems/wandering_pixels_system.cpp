#include "augs/math/steering.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/detail/visible_entities.h"

#include "view/audiovisual_state/systems/wandering_pixels_system.h"

void wandering_pixels_system::clear() {
	per_entity_cache.clear();
}

void wandering_pixels_system::clear_dead_entities(const cosmos& new_cosmos) {
	std::vector<unversioned_entity_id> to_erase;

	for (const auto it : per_entity_cache) {
		if (new_cosmos[it.first].dead()) {
			to_erase.push_back(it.first);
		}
	}

	for (const auto it : to_erase) {
		per_entity_cache.erase(it);
	}
}

wandering_pixels_system::cache& wandering_pixels_system::get_cache(const const_entity_handle id) {
	return per_entity_cache[id.get_id()];
}

const wandering_pixels_system::cache& wandering_pixels_system::get_cache(const const_entity_handle id) const {
	return per_entity_cache.at(id.get_id());
}

void wandering_pixels_system::advance_for(
	const visible_entities& visible,
	const cosmos& cosm,
	const augs::delta dt
) {
	global_time_seconds += dt.in_seconds();

	for (const auto e : visible.per_layer[render_layer::WANDERING_PIXELS_EFFECTS]) {
		advance_for(cosm[e], dt);
	}

	for (const auto e : visible.per_layer[render_layer::DIM_WANDERING_PIXELS_EFFECTS]) {
		advance_for(cosm[e], dt);
	}
}

void wandering_pixels_system::advance_for(
	const const_entity_handle handle, 
	const augs::delta dt
) {
	const auto dt_secs = dt.in_seconds();
	const auto dt_ms = dt.in_milliseconds();

	thread_local randomization rng;

	handle.dispatch_on_having<invariants::wandering_pixels>([&](const auto it) {
		auto& cache = get_cache(it);
		auto& used_rng = rng;

		const auto& wandering = it.template get<components::wandering_pixels>();
		const auto& wandering_def = it.template get<invariants::wandering_pixels>();

		const auto& cosm = it.get_cosmos();

		const auto anim = cosm.get_logical_assets().find(wandering_def.animation_id);

		if (anim == nullptr) {
			return;
		}

		const auto total_animation_duration = ::get_total_duration(anim->frames);

		if (cache.recorded_particle_count != wandering.particles_count) {
			cache.particles.resize(wandering.particles_count);
			cache.recorded_particle_count = wandering.particles_count;
		}

		const auto current_reach = xywh(*it.find_aabb());

		if (cache.recorded_reach != current_reach) {
			/* refresh_cache */ 
			for (auto& p : cache.particles) {
				p.pos.set(
					current_reach.x + used_rng.randval(0u, static_cast<unsigned>(current_reach.w)), 
					current_reach.y + used_rng.randval(0u, static_cast<unsigned>(current_reach.h))
				);

				p.current_lifetime_ms = used_rng.randval(0.f, total_animation_duration);
			}

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

			const auto sin_secs = static_cast<float>(sin(p.current_lifetime_ms / 1000));
			const auto cos_secs = static_cast<float>(cos(p.current_lifetime_ms / 1000));

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

			if (wandering.keep_particles_within_bounds) {
				auto velocity = p.pos - prev_pos;

				const auto tip_pos = p.pos;
				const auto bound = current_reach;
				const auto edges = bound.make_edges();

				auto min_dist = std::numeric_limits<real32>::max();

				for (auto& e : edges) {
					const auto dist = tip_pos.distance_from_segment_sq(e[0], e[1]);

					if (dist < min_dist) {
						min_dist = dist;
					}
				}

				min_dist = std::sqrt(min_dist);

				/* Finally, correct velocities against the walls */

				auto dir_mult = std::max(0.f, 1.f - min_dist / 30.f);

				if (!bound.hover(tip_pos)) {
					/* Protect from going outside */
					dir_mult = 1.f;
				}

				velocity +=
					augs::seek(
						velocity,
						p.pos,
						current_reach.get_center(),
						velocity.length()
					) * dir_mult
				;

				p.pos += velocity;
			}
		}
	});
}