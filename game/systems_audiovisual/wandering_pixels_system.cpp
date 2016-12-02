#include "wandering_pixels_system.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

void wandering_pixels_system::resample_state_for_audiovisuals(const cosmos& new_cosmos) {
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

wandering_pixels_system::cache& wandering_pixels_system::get_cache(const const_entity_handle id) {
	return per_entity_cache[id.get_id()];
}

const wandering_pixels_system::cache& wandering_pixels_system::get_cache(const const_entity_handle id) const {
	return per_entity_cache.at(id.get_id());
}

void wandering_pixels_system::advance_wandering_pixels_for(const const_entity_handle it, const float global_time, const augs::delta dt) {
	auto& cache = get_cache(it);
	const auto& wandering = it.get<components::wandering_pixels>();


	if (cache.recorded_component.count != wandering.count) {
		cache.particles.resize(wandering.count);
		cache.recorded_component.count = wandering.count;
	}

	if (!(cache.recorded_component.reach == wandering.reach)) {
		for (auto& p : cache.particles) {
			p.pos.set(wandering.reach.x + cache.generator() % unsigned(wandering.reach.w) , wandering.reach.y + cache.generator() % unsigned(wandering.reach.h));
		}

		cache.recorded_component.reach = wandering.reach;
	}

	constexpr unsigned max_direction_time = 4000u;
	constexpr float interp_time = 700.f;

	for (auto& p : cache.particles) {
		if (p.direction_ms_left <= 0.f) {
			p.direction_ms_left = cache.generator() % max_direction_time + 800;

			p.current_direction = p.current_direction.perpendicular_cw();

			const auto dir = p.current_direction;
			const auto reach = wandering.reach;

			float chance_to_flip = 0.f;

			if (dir.x > 0) {
				chance_to_flip = (p.pos.x - reach.x) / reach.w;
			}
			else if (dir.x < 0) {
				chance_to_flip = 1.0 - (p.pos.x - reach.x) / reach.w;
			}
			if (dir.y > 0) {
				chance_to_flip = (p.pos.y - reach.y) / reach.h;
			}
			else if (dir.y < 0) {
				chance_to_flip = 1.0 - (p.pos.y - reach.y) / reach.h;
			}

			if (chance_to_flip < 0) {
				chance_to_flip = 0;
			}

			if (chance_to_flip > 1) {
				chance_to_flip = 1;
			}

			if (cache.generator() % 100u <= chance_to_flip * 100.f) {
				p.current_direction = -p.current_direction;
			}
		}
		else {
			p.direction_ms_left -= dt.in_milliseconds();
		}

		vec2 considered_direction;

		if (p.direction_ms_left <= interp_time) {
			considered_direction = augs::interp(vec2(), p.current_direction, p.direction_ms_left / interp_time);
		}
		else if (p.direction_ms_left >= max_direction_time - interp_time) {
			considered_direction = augs::interp(vec2(), p.current_direction, (max_direction_time - p.direction_ms_left) / interp_time);
		}
		else
		{
			considered_direction = p.current_direction;
		}

		p.pos += considered_direction * 20 * dt.in_seconds();
		if (considered_direction.x > 0) {
			p.pos.y += considered_direction.x * sin(global_time) * 20 * dt.in_seconds() * 1.2;
		}
		else if (considered_direction.x < 0) {
			p.pos.y -= -considered_direction.x * sin(global_time) * 20 * dt.in_seconds() * 1.2;
		}
		if (considered_direction.y > 0) {
			p.pos.x += considered_direction.y * cos(global_time) * 20 * dt.in_seconds() * 1.2;
		}
		else if (considered_direction.y < 0) {
			p.pos.x -= -considered_direction.y * cos(global_time) * 20 * dt.in_seconds() * 1.2;
		}

		//p.pos.x += cos(global_time) * 20 * dt.in_seconds() * 1.2;
	}
}

void wandering_pixels_system::draw_wandering_pixels_for(const const_entity_handle it, const drawing_input& in) const {
	const auto& wandering = it.get<components::wandering_pixels>();
	const auto& cache = get_cache(it);

	components::sprite::drawing_input pixel_input(in.target_buffer);
	pixel_input.camera = in.camera;

	for (const auto& p : cache.particles) {
		pixel_input.renderable_transform = p.pos;

		wandering.face.draw(pixel_input);
	}
}