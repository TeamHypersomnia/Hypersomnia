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
}

void wandering_pixels_system::advance_for(
	const const_entity_handle handle, 
	const augs::delta dt
) {
	const auto dt_secs = dt.in_seconds();
	const auto dt_ms = dt.in_milliseconds();

	thread_local randomization rng;

	handle.dispatch_on_having<invariants::wandering_pixels>([&](const auto it) {
		const auto& cosmos = it.get_cosmos();
		auto& cache = get_cache(it);
		auto& used_rng = rng;

		const auto& wandering = it.template get<components::wandering_pixels>();
		const auto& wandering_def = it.template get<invariants::wandering_pixels>();

		if (cache.recorded_component.particles_count != wandering.particles_count) {
			cache.particles.resize(wandering.particles_count);
			cache.recorded_component.particles_count = wandering.particles_count;
		}

		const auto new_reach = wandering.get_reach();

		if (cache.recorded_component.get_reach() != new_reach) {
			/* refresh_cache */ 
			for (auto& p : cache.particles) {
				p.pos.set(
					new_reach.x + rng.randval(0u, static_cast<unsigned>(new_reach.w)), 
					new_reach.y + rng.randval(0u, static_cast<unsigned>(new_reach.h))
				);

				p.current_lifetime_ms = rng.randval(0.f, wandering_def.frame_duration_ms);
			}

			cache.recorded_component.set_reach(new_reach);
		}

		constexpr unsigned max_direction_time = 4000u;
		constexpr float interp_time = 700.f;

		for (auto& p : cache.particles) {
			p.current_lifetime_ms += dt_ms + dt_ms * (p.direction_ms_left / max_direction_time) + dt_ms * p.current_direction.radians();

			if (p.direction_ms_left <= 0.f) {
				p.direction_ms_left = static_cast<float>(rng.randval(max_direction_time, max_direction_time + 800u));

				p.current_direction = p.current_direction.perpendicular_cw();

				const auto dir = p.current_direction;
				const auto reach = new_reach;

				float chance_to_flip = 0.f;

				p.current_velocity = static_cast<float>(rng.randval(3, 3 + 35));

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

				if (rng.randval(0u, 100u) <= chance_to_flip * 100.f) {
					p.current_direction = -p.current_direction;
				}
			}
			else {
				p.direction_ms_left -= dt_ms;
			}

			vec2 considered_direction;

			if (p.direction_ms_left <= interp_time) {
				considered_direction = augs::interp(vec2(), p.current_direction, p.direction_ms_left / interp_time);
			}
			else if (p.direction_ms_left >= max_direction_time - interp_time) {
				considered_direction = augs::interp(vec2(), p.current_direction, (max_direction_time - p.direction_ms_left) / interp_time);
			}
			else {
				considered_direction = p.current_direction;
			}

			const auto vel = p.current_velocity;

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
		}
	});
}