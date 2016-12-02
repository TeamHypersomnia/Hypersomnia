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

void wandering_pixels_system::advance_wandering_pixels_for(const const_entity_handle it, const augs::delta dt) {
	auto& cache = get_cache(it);
	const auto& wandering = it.get<components::wandering_pixels>();

	const bool should_rebuild_cache = 
		!(cache.recorded_component.reach == wandering.reach)
		|| cache.recorded_component.count != wandering.count;


	if (should_rebuild_cache) {
		cache.particles.resize(wandering.count);
	}

	for (auto& p : cache.particles) {
		static const vec2 offsets[4] = {
			{ 1, 0 },
			{ 0, 1 },
			{ -1, 0 },
			{ 0, -1 }
		};

		const auto offset = cache.generator() % 4u;

		p.pos += offsets[offset] * 10 * dt.in_seconds();
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